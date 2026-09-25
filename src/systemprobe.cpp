#include "systemprobe.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QStorageInfo>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QIODevice>
#include <QTimer>
#include <QVariantList>

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDBusVariant>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QStandardPaths>

#include <QtMath>
#include <QQuickWindow>
#include <QImage>
#include <cmath>
#include <csignal>

namespace {

QString readTrimmed(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    return QString::fromLocal8Bit(f.readAll()).trimmed();
}

/*
 * Splits a file into lines.
 *
 * procfs reports a size of 0, so QFile::atEnd() is already true before the
 * first read and a "while (!f.atEnd())" loop over /proc/stat or
 * /proc/meminfo would simply never execute. Read to EOF instead.
 */
QStringList readLines(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QStringList();
    const QStringList lines = QString::fromLocal8Bit(f.readAll())
            .split(QLatin1Char('\n'));
    f.close();
    return lines;
}

/*
 * Parsers for "gdbus call" output.
 *
 * D-Bus property maps are fetched with gdbus instead of Qt because this Qt
 * demarshals a{sv} replies badly: keys came back empty, and reading the
 * values as QVariant crashed inside libdbus. gdbus prints the dictionary as
 * one GVariant literal, e.g.
 *
 *   ({'Name': <'Wired'>, 'Powered': <true>, 'State': <'online'>},)
 *
 * so a small scanner over that text is both safe and complete. Anything we
 * do not understand simply yields no entry, which the UI shows as "unknown".
 */

void skipSpace(const QString &s, int *i)
{
    while (*i < s.size() && s.at(*i).isSpace())
        ++(*i);
}

// Reads a GVariant single-quoted string; *i ends up past the closing quote.
bool readQuoted(const QString &s, int *i, QString *out)
{
    if (*i >= s.size() || s.at(*i) != QLatin1Char('\''))
        return false;
    ++(*i);
    QString value;
    while (*i < s.size()) {
        const QChar c = s.at(*i);
        if (c == QLatin1Char('\\')) {
            ++(*i);
            if (*i >= s.size())
                return false;
            value += s.at(*i);
            ++(*i);
            continue;
        }
        if (c == QLatin1Char('\'')) {
            ++(*i);
            *out = value;
            return true;
        }
        value += c;
        ++(*i);
    }
    return false;
}

QVariant readGVariant(const QString &s, int *i);

// {...} dict, [...] array or (...) tuple; *i sits on the opening bracket.
QVariant readGContainer(const QString &s, int *i, QChar close)
{
    ++(*i);
    QVariantList list;
    for (;;) {
        skipSpace(s, i);
        if (*i >= s.size())
            return QVariant();
        if (s.at(*i) == close) {
            ++(*i);
            break;
        }
        const QVariant v = readGVariant(s, i);
        if (!v.isValid())
            return QVariant();
        list.append(v);
        skipSpace(s, i);
        if (*i < s.size() && s.at(*i) == QLatin1Char(',')) {
            ++(*i);
            continue;
        }
        skipSpace(s, i);
        if (*i < s.size() && s.at(*i) == close) {
            ++(*i);
            break;
        }
        return QVariant();
    }
    if (close == QLatin1Char('}')) {
        // a dict entry is a (key, value) pair
        if (list.size() == 2)
            return list.at(1);
        return QVariant();
    }
    return list;
}

QVariant readGVariant(const QString &s, int *i)
{
    skipSpace(s, i);
    if (*i >= s.size())
        return QVariant();

    const QChar c = s.at(*i);
    if (c == QLatin1Char('\'')) {
        QString str;
        if (!readQuoted(s, i, &str))
            return QVariant();
        return str;
    }
    if (c == QLatin1Char('<')) {              // variant wrapper
        ++(*i);
        const QVariant v = readGVariant(s, i);
        skipSpace(s, i);
        if (*i < s.size() && s.at(*i) == QLatin1Char('>'))
            ++(*i);
        return v;
    }
    if (c == QLatin1Char('@')) {              // type annotation, e.g. @as
        ++(*i);
        while (*i < s.size() && !s.at(*i).isSpace()
               && s.at(*i) != QLatin1Char('[') && s.at(*i) != QLatin1Char('(')
               && s.at(*i) != QLatin1Char('<') && s.at(*i) != QLatin1Char('\''))
            ++(*i);
        return readGVariant(s, i);
    }
    if (c == QLatin1Char('['))
        return readGContainer(s, i, QLatin1Char(']'));
    if (c == QLatin1Char('('))
        return readGContainer(s, i, QLatin1Char(')'));

    // bare token: true/false, a number, or an identifier such as a handle
    QString token;
    while (*i < s.size()) {
        const QChar ch = s.at(*i);
        if (ch == QLatin1Char(',') || ch == QLatin1Char(')') || ch == QLatin1Char(']')
                || ch == QLatin1Char('}') || ch == QLatin1Char('>') || ch.isSpace())
            break;
        token += ch;
        ++(*i);
    }
    if (token == QLatin1String("true"))
        return true;
    if (token == QLatin1String("false"))
        return false;
    bool ok = false;
    if (token.contains(QLatin1Char('.'))) {
        const double d = token.toDouble(&ok);
        if (ok)
            return d;
    } else {
        const qlonglong n = token.toLongLong(&ok);
        if (ok)
            return n;
    }
    if (token.isEmpty())
        return QVariant();
    return token;
}

// Parses the top level dictionary of a gdbus call reply.
QVariantMap parseGdbusMap(const QString &text)
{
    QVariantMap map;
    QString s = text;
    s.remove(QLatin1Char('\n'));
    s.remove(QLatin1Char('\r'));
    s.remove(QLatin1Char('\t'));

    int i = s.indexOf(QLatin1Char('{'));
    if (i < 0)
        return map;
    ++i;

    for (;;) {
        skipSpace(s, &i);
        if (i >= s.size() || s.at(i) == QLatin1Char('}'))
            break;
        QString key;
        if (!readQuoted(s, &i, &key))
            break;
        skipSpace(s, &i);
        if (i >= s.size() || s.at(i) != QLatin1Char(':'))
            break;
        ++i;
        const QVariant value = readGVariant(s, &i);
        if (!value.isValid())
            break;
        map.insert(key, value);
        skipSpace(s, &i);
        if (i < s.size() && s.at(i) == QLatin1Char(',')) {
            ++i;
            continue;
        }
        break;
    }
    return map;
}

bool readNumber(const QString &path, double *out)
{
    const QString text = readTrimmed(path);
    if (text.isEmpty())
        return false;
    bool ok = false;
    const double value = text.toDouble(&ok);
    if (!ok)
        return false;
    *out = value;
    return true;
}

QList<qint64> numbers(const QString &line)
{
    QList<qint64> out;
    const QStringList parts = line.simplified().split(' ');
    for (int i = 0; i < parts.size(); ++i) {
        bool ok = false;
        const qint64 v = parts.at(i).toLongLong(&ok);
        if (ok)
            out.append(v);
    }
    return out;
}

// Set from SIGUSR1 (kill -USR1) and consumed by the next tick, so no Qt work
// ever happens inside the handler itself. Grabs the window to /tmp/app-shot.png
// for visual verification over SSH - /dev/fb0 is blank and the VirtualBox
// screenshot is scrambled on this build.
volatile sig_atomic_t g_shotRequested = 0;
void requestShot(int) { g_shotRequested = 1; }

double percentChange(qint64 nowTotal, qint64 nowIdle, qint64 prevTotal, qint64 prevIdle)
{
    const qint64 dTotal = nowTotal - prevTotal;
    const qint64 dIdle = nowIdle - prevIdle;
    if (dTotal <= 0)
        return -1.0;
    const double busy = double(dTotal - dIdle);
    double pct = 100.0 * busy / double(dTotal);
    if (pct < 0.0)
        pct = 0.0;
    if (pct > 100.0)
        pct = 100.0;
    return pct;
}

} // namespace

SystemProbe::SystemProbe(QObject *parent)
    : QObject(parent)
    , m_dirty(false)
    , m_ticks(0)
    , m_window(0)
    , m_gdbusChecked(false)
    , m_prevCpuTotal(-1)
    , m_prevCpuIdle(-1)
    , m_prevIoRead(-1)
    , m_prevIoWrite(-1)
    , m_prevRx(-1)
    , m_prevTx(-1)
    , m_audio(0)
    , m_audioDev(0)
    , m_micActive(false)
    , m_lastMicEmit(0)
{
    refreshSystem();

    signal(SIGUSR1, requestShot);

    QTimer *timer = new QTimer(this);
    timer->setInterval(2000);
    connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
    timer->start();

    tick();
}

// ---------------------------------------------------------------- invokables

QString SystemProbe::readFile(const QString &path) const
{
    return readTrimmed(path);
}

QStringList SystemProbe::listDir(const QString &path) const
{
    QDir dir(path);
    if (!dir.exists())
        return QStringList();
    QStringList entries = dir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
                                        QDir::Name);
    return entries;
}

void SystemProbe::refresh()
{
    tick();
}

void SystemProbe::set(const QString &key, const QVariant &value)
{
    if (m_values.value(key) == value)
        return;
    m_values.insert(key, value);
    m_dirty = true;
}

void SystemProbe::unset(const QString &key)
{
    if (m_values.remove(key) > 0)
        m_dirty = true;
}

void SystemProbe::tick()
{
    const QDateTime now = QDateTime::currentDateTime();
    double elapsed = 2.0;
    if (m_lastTick.isValid())
        elapsed = m_lastTick.msecsTo(now) / 1000.0;
    if (elapsed <= 0.0)
        elapsed = 1.0;
    m_lastTick = now;

    refreshCpu();
    refreshMemory();
    refreshStorage();
    refreshPower();
    refreshThermal();
    refreshDisplay();

    if (m_ticks % 4 == 0)
        refreshDbus();
    // The property maps are read from the gdbus cache, so these stay cheap
    // and run every tick: the first answer lands well before tick 1.
    refreshConnman();
    refreshOfono();
    refreshBluez();
    ++m_ticks;

    if (g_shotRequested) {
        g_shotRequested = 0;
        if (m_window) {
            const QImage img = m_window->grabWindow();
            const QString path = QLatin1String("/tmp/app-shot.png");
            if (!img.isNull() && img.save(path))
                qInfo("SystemProbe: saved %s (%dx%d)",
                      qPrintable(path), img.width(), img.height());
            else
                qWarning("SystemProbe: window grab failed");
            QStringList keys = m_values.keys();
            keys.sort();
            qInfo("KEYS %s", qPrintable(keys.join(QLatin1Char(','))));
        }
    }

    if (m_dirty) {
        m_dirty = false;
        emit valuesChanged();
    }

    Q_UNUSED(elapsed);
}

// ------------------------------------------------------------- file readers

void SystemProbe::refreshCpu()
{
    // /proc/stat: "cpu  user nice system idle iowait irq softirq steal"
    const QStringList statLines = readLines(QLatin1String("/proc/stat"));
    if (!statLines.isEmpty()) {
        int core = -1;
        for (int li = 0; li < statLines.size(); ++li) {
            const QString line = statLines.at(li).trimmed();
            if (line.startsWith(QLatin1String("cpu"))) {
                const QList<qint64> n = numbers(line);
                if (n.size() < 5)
                    continue;
                qint64 total = 0;
                for (int i = 1; i < n.size() && i < 9; ++i)
                    total += n.at(i);
                const qint64 idle = n.at(4) + (n.size() > 5 ? n.at(5) : 0);

                if (line.startsWith(QLatin1String("cpu "))) {
                    if (m_prevCpuTotal >= 0) {
                        const double pct = percentChange(total, idle, m_prevCpuTotal, m_prevCpuIdle);
                        if (pct >= 0.0)
                            set(QLatin1String("cpu.total"), pct);
                    }
                    m_prevCpuTotal = total;
                    m_prevCpuIdle = idle;
                } else {
                    ++core;
                    if (core < m_prevCoreTotal.size() && m_prevCoreTotal.at(core) >= 0) {
                        const double pct = percentChange(total, idle,
                                                         m_prevCoreTotal.at(core),
                                                         m_prevCoreIdle.at(core));
                        if (pct >= 0.0)
                            set(QString("cpu.c%1").arg(core), pct);
                    }
                    if (core >= m_prevCoreTotal.size()) {
                        m_prevCoreTotal.append(total);
                        m_prevCoreIdle.append(idle);
                    } else {
                        m_prevCoreTotal[core] = total;
                        m_prevCoreIdle[core] = idle;
                    }
                }
            }
        }
        set(QLatin1String("cpu.coreCount"), m_prevCoreTotal.size());
    }

    const QString load = readTrimmed("/proc/loadavg");
    if (!load.isEmpty()) {
        const QStringList parts = load.split(' ');
        if (parts.size() >= 3) {
            set(QLatin1String("load.1"), parts.at(0).toDouble());
            set(QLatin1String("load.5"), parts.at(1).toDouble());
            set(QLatin1String("load.15"), parts.at(2).toDouble());
        }
    }

    const QString uptime = readTrimmed("/proc/uptime");
    if (!uptime.isEmpty())
        set(QLatin1String("uptime"), uptime.split(' ').value(0).toDouble());
}

void SystemProbe::refreshMemory()
{
    const QStringList memLines = readLines(QLatin1String("/proc/meminfo"));
    if (memLines.isEmpty())
        return;

    double total = -1, available = -1, free = -1, buffers = -1, cached = -1;
    double swapTotal = -1, swapFree = -1;
    for (int li = 0; li < memLines.size(); ++li) {
        const QString line = memLines.at(li);
        const int colon = line.indexOf(':');
        if (colon < 0)
            continue;
        const QString key = line.left(colon);
        const double kb = line.mid(colon + 1).simplified().split(' ').value(0).toDouble();
        if (key == QLatin1String("MemTotal")) total = kb;
        else if (key == QLatin1String("MemAvailable")) available = kb;
        else if (key == QLatin1String("MemFree")) free = kb;
        else if (key == QLatin1String("Buffers")) buffers = kb;
        else if (key == QLatin1String("Cached")) cached = kb;
        else if (key == QLatin1String("SwapTotal")) swapTotal = kb;
        else if (key == QLatin1String("SwapFree")) swapFree = kb;
    }

    if (total > 0) {
        if (available < 0)
            available = free + buffers + cached;
        set(QLatin1String("mem.total"), total * 1024.0);
        set(QLatin1String("mem.available"), available * 1024.0);
        set(QLatin1String("mem.used"), (total - available) * 1024.0);
        set(QLatin1String("mem.usedPct"), 100.0 * (total - available) / total);
    }
    if (swapTotal > 0) {
        set(QLatin1String("mem.swapTotal"), swapTotal * 1024.0);
        set(QLatin1String("mem.swapUsed"), (swapTotal - swapFree) * 1024.0);
    }
}

void SystemProbe::refreshStorage()
{
    const char *roots[][2] = { { "/", "root" }, { "/home", "home" } };
    for (int i = 0; i < 2; ++i) {
        QStorageInfo info(QString::fromLatin1(roots[i][0]));
        if (!info.isValid() || !info.isReady())
            continue;
        const QString p = QString("disk.%1.").arg(QLatin1String(roots[i][1]));
        set(p + QLatin1String("total"), double(info.bytesTotal()));
        set(p + QLatin1String("free"), double(info.bytesFree()));
        set(p + QLatin1String("available"), double(info.bytesAvailable()));
    }

    // /proc/diskstats: major minor name reads ... sectors_read ... writes ... sectors_written
    qint64 read = 0, written = 0;
    const QStringList stats = readLines(QLatin1String("/proc/diskstats"));
    for (int li = 0; li < stats.size(); ++li) {
        const QStringList p = stats.at(li).simplified().split(' ');
        if (p.size() < 10)
            continue;
        const QString name = p.at(2);
        if (name.startsWith(QLatin1String("loop"))
                || name.startsWith(QLatin1String("ram"))
                || name.startsWith(QLatin1String("zram")))
            continue;
        read += p.at(5).toLongLong();
        written += p.at(9).toLongLong();
    }

    if (m_prevIoRead >= 0 && m_lastTick.isValid()) {
        // 512-byte sectors -> KiB/s, over the tick interval (nominally 2 s)
        const double secs = 2.0;
        set(QLatin1String("disk.io.readKbs"), double(read - m_prevIoRead) * 512.0 / 1024.0 / secs);
        set(QLatin1String("disk.io.writeKbs"), double(written - m_prevIoWrite) * 512.0 / 1024.0 / secs);
    }
    m_prevIoRead = read;
    m_prevIoWrite = written;
}

void SystemProbe::refreshPower()
{
    const QString base = QLatin1String("/sys/class/power_supply/BAT0/");
    if (!QFile::exists(base)) {
        set(QLatin1String("bat.present"), false);
        unset(QLatin1String("bat.capacity"));
        return;
    }
    set(QLatin1String("bat.present"), true);

    double v = 0;
    if (readNumber(base + QLatin1String("capacity"), &v))
        set(QLatin1String("bat.capacity"), v);
    const QString status = readTrimmed(base + QLatin1String("status"));
    if (!status.isEmpty())
        set(QLatin1String("bat.status"), status);

    double now = 0, full = 0, design = 0;
    const bool hasNow = readNumber(base + QLatin1String("energy_now"), &now);
    const bool hasFull = readNumber(base + QLatin1String("energy_full"), &full);
    const bool hasDesign = readNumber(base + QLatin1String("energy_full_design"), &design);
    if (hasNow)
        set(QLatin1String("bat.energyNow"), now / 1000.0); // uWh -> mWh
    if (hasFull && hasDesign && design > 0)
        set(QLatin1String("bat.health"), 100.0 * full / design);

    if (readNumber(base + QLatin1String("voltage_now"), &v))
        set(QLatin1String("bat.voltage"), v / 1000000.0);
    if (readNumber(base + QLatin1String("current_now"), &v))
        set(QLatin1String("bat.current"), v / 1000000.0);
    if (readNumber(base + QLatin1String("power_now"), &v))
        set(QLatin1String("bat.power"), v / 1000000.0);
    if (readNumber(base + QLatin1String("cycle_count"), &v))
        set(QLatin1String("bat.cycles"), v);
    if (readNumber(base + QLatin1String("temp"), &v))
        set(QLatin1String("bat.temp"), v / 10.0);

    const QString tech = readTrimmed(base + QLatin1String("technology"));
    if (!tech.isEmpty())
        set(QLatin1String("bat.technology"), tech);

    double ac = -1;
    if (readNumber(QLatin1String("/sys/class/power_supply/AC/online"), &ac))
        set(QLatin1String("bat.charger"), ac > 0);
    const QString acType = readTrimmed(QLatin1String("/sys/class/power_supply/AC/type"));
    if (!acType.isEmpty())
        set(QLatin1String("bat.chargerType"), acType);
}

void SystemProbe::refreshThermal()
{
    const QString root = QLatin1String("/sys/class/thermal");
    QDir dir(root);
    const QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    int zones = 0, cool = 0;
    for (int i = 0; i < entries.size(); ++i) {
        const QString path = root + QLatin1String("/") + entries.at(i);
        if (entries.at(i).startsWith(QLatin1String("thermal_zone"))) {
            if (zones >= 8)
                continue;
            const QString type = readTrimmed(path + QLatin1String("/type"));
            double temp = 0;
            if (readNumber(path + QLatin1String("/temp"), &temp)) {
                set(QString("thermal.z%1.type").arg(zones), type);
                set(QString("thermal.z%1.temp").arg(zones), temp / 1000.0);
                ++zones;
            }
        } else if (entries.at(i).startsWith(QLatin1String("cooling_device"))) {
            if (cool >= 8)
                continue;
            double cur = -1, max = -1;
            readNumber(path + QLatin1String("/cur_state"), &cur);
            readNumber(path + QLatin1String("/max_state"), &max);
            if (cur >= 0 && max >= 0) {
                const QString name = readTrimmed(path + QLatin1String("/type"));
                set(QString("thermal.c%1.name").arg(cool), name);
                set(QString("thermal.c%1.cur").arg(cool), cur);
                set(QString("thermal.c%1.max").arg(cool), max);
                ++cool;
            }
        }
    }
    set(QLatin1String("thermal.zoneCount"), zones);
    set(QLatin1String("thermal.coolCount"), cool);
}

void SystemProbe::refreshDisplay()
{
    QDir dir(QLatin1String("/sys/class/backlight"));
    const QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    if (!entries.isEmpty()) {
        const QString path = QLatin1String("/sys/class/backlight/") + entries.first();
        double cur = -1, max = -1;
        if (!readNumber(path + QLatin1String("/brightness"), &cur))
            readNumber(path + QLatin1String("/actual_brightness"), &cur);
        readNumber(path + QLatin1String("/max_brightness"), &max);
        if (cur >= 0)
            set(QLatin1String("disp.brightness"), cur);
        if (max > 0)
            set(QLatin1String("disp.maxBrightness"), max);
    }

    // The display state itself comes from MCE - see refreshMce(), which runs
    // on the slower D-Bus cadence.
}

void SystemProbe::refreshSystem()
{
    const QString osRelease = readTrimmed(QLatin1String("/etc/os-release"));
    if (!osRelease.isEmpty()) {
        const QStringList lines = osRelease.split('\n');
        for (int i = 0; i < lines.size(); ++i) {
            const QString line = lines.at(i);
            if (line.startsWith(QLatin1String("NAME=")))
                set(QLatin1String("sys.os"), line.mid(5).remove('"'));
            else if (line.startsWith(QLatin1String("VERSION=")))
                set(QLatin1String("sys.version"), line.mid(8).remove('"'));
            else if (line.startsWith(QLatin1String("PRETTY_NAME=")))
                set(QLatin1String("sys.prettyName"), line.mid(12).remove('"'));
        }
    }

    const QString hw = readTrimmed(QLatin1String("/etc/hw-release"));
    if (!hw.isEmpty())
        set(QLatin1String("sys.device"), hw.split('\n').first());

    const QString version = readTrimmed(QLatin1String("/proc/version"));
    if (!version.isEmpty()) {
        const QStringList p = version.split(' ');
        if (p.size() >= 3)
            set(QLatin1String("sys.kernel"), p.at(2));
    }

    const QString hostname = readTrimmed(QLatin1String("/proc/sys/kernel/hostname"));
    if (!hostname.isEmpty())
        set(QLatin1String("sys.hostname"), hostname);

    const QString bootId = readTrimmed(QLatin1String("/proc/sys/kernel/random/boot_id"));
    if (!bootId.isEmpty())
        set(QLatin1String("sys.bootId"), bootId);

    const QString machine = readTrimmed(QLatin1String("/proc/sys/kernel/arch"));
    Q_UNUSED(machine);
}

// -------------------------------------------------------------- D-Bus reads

// Cache key for one (service, path, interface, method) property read.
static QString mapKey(const QString &service, const QString &path,
                      const QString &iface, const QString &method)
{
    return service + QLatin1Char('|') + path + QLatin1Char('|')
            + iface + QLatin1Char('|') + method;
}

void SystemProbe::startMapCall(const QString &service, const QString &path,
                               const QString &iface, const QString &method,
                               const QStringList &args)
{
    const QString key = mapKey(service, path, iface, method);
    if (m_pending.values().contains(key))
        return;                                  // already in flight

    if (!m_gdbusChecked) {
        m_gdbusChecked = true;
        m_gdbus = QStandardPaths::findExecutable(QLatin1String("gdbus"));
        if (m_gdbus.isEmpty())
            qWarning("SystemProbe: gdbus not found, D-Bus property maps unavailable");
    }
    if (m_gdbus.isEmpty())
        return;

    QStringList a;
    a << QLatin1String("call") << QLatin1String("--system")
      << QLatin1String("--timeout") << QLatin1String("2")
      << QLatin1String("--dest") << service
      << QLatin1String("--object-path") << path
      << QLatin1String("--method") << (iface + QLatin1Char('.') + method)
      << args;

    QProcess *p = new QProcess(this);
    m_pending.insert(p, key);
    connect(p, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &SystemProbe::mapCallFinished);
    connect(p, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            this, &SystemProbe::mapCallError);
    p->start(m_gdbus, a);
}

void SystemProbe::mapCallFinished(int exitCode, QProcess::ExitStatus status)
{
    QProcess *p = qobject_cast<QProcess *>(sender());
    if (!p || !m_pending.contains(p))
        return;
    const QString key = m_pending.take(p);
    const QByteArray out = p->readAllStandardOutput();
    p->deleteLater();

    if (status == QProcess::NormalExit && exitCode == 0) {
        const QVariantMap map = parseGdbusMap(QString::fromUtf8(out));
        m_mapCache.insert(key, map);
        if (!map.isEmpty())
            qInfo("PROBE %s -> %d keys", qPrintable(key), map.size());
    } else {
        // Object absent or refused by policy: an empty map is the right
        // answer, and it also records that we did get an answer.
        m_mapCache.insert(key, QVariantMap());
    }
}

void SystemProbe::mapCallError(QProcess::ProcessError)
{
    QProcess *p = qobject_cast<QProcess *>(sender());
    if (!p || !m_pending.contains(p))
        return;
    m_mapCache.insert(m_pending.take(p), QVariantMap());
    p->deleteLater();
}

QVariantMap SystemProbe::dbusProps(const QString &service, const QString &path,
                                   const QString &iface) const
{
    // Sailfish's connman exposes no org.freedesktop.DBus.Properties on its
    // technology objects, so the per-interface GetProperties() is the way in.
    return m_mapCache.value(mapKey(service, path, iface,
                                   QLatin1String("GetProperties")));
}

QVariantMap SystemProbe::dbusGetAll(const QString &service, const QString &path,
                                    const QString &iface) const
{
    return m_mapCache.value(mapKey(service, path, iface,
                                   QLatin1String("GetAll")));
}

QVariant SystemProbe::dbusCall(const QString &service, const QString &path,
                               const QString &iface, const QString &method) const
{
    QDBusInterface bus(service, path, iface, QDBusConnection::systemBus());
    if (!bus.isValid())
        return QVariant();
    bus.setTimeout(800);
    const QDBusMessage reply = bus.call(QDBus::Block, method);
    if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty())
        return QVariant();
    return reply.arguments().first();
}

QString SystemProbe::prop(const QVariantMap &m, const QString &key)
{
    return m.value(key).toString();
}

void SystemProbe::refreshDbus()
{
    // Fire off the async property-map reads first: gdbus answers in ~10 ms
    // and the replies are picked up by the refreshers on the next tick.
    static const char *const techPaths[] = {
        "/net/connman/technology/ethernet",
        "/net/connman/technology/wifi",
        "/net/connman/technology/gps",
        "/net/connman/technology/cellular",
        "/net/connman/technology/bluetooth"
    };
    for (int i = 0; i < 5; ++i)
        startMapCall(QLatin1String("net.connman"), QLatin1String(techPaths[i]),
                     QLatin1String("net.connman.Technology"),
                     QLatin1String("GetProperties"));

    static const char *const modems[] = { "/ril_0", "/ril_1" };
    for (int i = 0; i < 2; ++i) {
        startMapCall(QLatin1String("org.ofono"), QLatin1String(modems[i]),
                     QLatin1String("org.ofono.NetworkRegistration"),
                     QLatin1String("GetProperties"));
        startMapCall(QLatin1String("org.ofono"), QLatin1String(modems[i]),
                     QLatin1String("org.ofono.SimManager"),
                     QLatin1String("GetProperties"));
    }

    startMapCall(QLatin1String("org.bluez"), QLatin1String("/org/bluez/hci0"),
                 QLatin1String("org.bluez.Adapter1"),
                 QLatin1String("GetAll"),
                 QStringList() << QLatin1String("org.bluez.Adapter1"));

    refreshMce();
}

void SystemProbe::refreshMce()
{
    // MCE filters property introspection (GetAll on the request interface is
    // rejected by the system bus policy) but answers its own request methods,
    // and MCE sits in the Base permission, so this works for every app.
    const QVariant state = dbusCall(QLatin1String("com.nokia.mce"),
                                    QLatin1String("/com/nokia/mce/request"),
                                    QLatin1String("com.nokia.mce.request"),
                                    QLatin1String("get_display_status"));
    if (state.isValid() && !state.toString().isEmpty())
        set(QLatin1String("disp.state"), state.toString());
    else
        unset(QLatin1String("disp.state"));
}

void SystemProbe::refreshConnman()
{
    QDBusInterface bus(QLatin1String("net.connman"), QLatin1String("/"),
                       QLatin1String("net.connman.Manager"),
                       QDBusConnection::systemBus());
    if (!bus.isValid()) {
        unset(QLatin1String("net.techCount"));
        return;
    }
    bus.setTimeout(800);

    // Technology objects live at well-known paths, so every read is a plain
    // GetAll -> QVariantMap. (Parsing the GetTechnologies/GetServices arrays
    // would need QDBusArgument demarshalling, which is what crashed us.)
    static const char *const techPaths[] = {
        "/net/connman/technology/ethernet",
        "/net/connman/technology/wifi",
        "/net/connman/technology/gps",
        "/net/connman/technology/cellular",
        "/net/connman/technology/bluetooth"
    };

    int count = 0;
    for (int i = 0; i < 5; ++i) {
        const QVariantMap tech = dbusProps(QLatin1String("net.connman"),
                                           QLatin1String(techPaths[i]),
                                           QLatin1String("net.connman.Technology"));
        if (tech.isEmpty())
            continue;
        ++count;

        const QString type = prop(tech, QLatin1String("Type"));
        const bool connected = tech.value(QLatin1String("Connected")).toBool();
        const bool powered = tech.value(QLatin1String("Powered")).toBool();
        const QString text = connected ? QObject::tr("connected")
                             : (powered ? QObject::tr("available")
                                        : QObject::tr("off"));
        const QString state = prop(tech, QLatin1String("State"));

        if (type == QLatin1String("ethernet")) {
            set(QLatin1String("net.eth.state"), text);
            if (!state.isEmpty())
                set(QLatin1String("net.eth.detail"), state);
        } else if (type == QLatin1String("wifi")) {
            set(QLatin1String("net.wifi.state"), text);
            if (!state.isEmpty())
                set(QLatin1String("net.wifi.detail"), state);
        } else if (type == QLatin1String("gps")) {
            set(QLatin1String("net.gps.power"), powered);
        } else if (type == QLatin1String("cellular")) {
            set(QLatin1String("net.cell.state"), text);
        }
    }
    set(QLatin1String("net.techCount"), count);

    // Signal strength from /proc/net/wireless, readable once the Internet
    // permission has put the interfaces into our namespace. The kernel
    // reports link quality out of 70.
    double link = -1;
    const QStringList wirelessLines = readLines(QLatin1String("/proc/net/wireless"));
    for (int li = 0; li < wirelessLines.size(); ++li) {
        const QStringList parts = wirelessLines.at(li).simplified().split(' ');
        if (parts.size() < 4 || !parts.at(0).endsWith(QLatin1Char(':')))
            continue;
        bool ok = false;
        const double quality = parts.at(2).toDouble(&ok);
        if (!ok)
            continue;
        if (quality > link)
            link = quality;
    }
    if (link >= 0) {
        const double pct = 100.0 * link / 70.0;
        set(QLatin1String("wifi.strength"), pct > 100.0 ? 100.0 : pct);
        set(QLatin1String("wifi.link"), link);
    } else {
        unset(QLatin1String("wifi.strength"));
        unset(QLatin1String("wifi.link"));
    }

    // Interface counters (only reachable with the Internet permission, which
    // lifts "net none" and puts the interfaces into our namespace).
    QDir netdir(QLatin1String("/sys/class/net"));
    const QStringList ifaces = netdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    qint64 rx = 0, tx = 0;
    int real = 0;
    for (int i = 0; i < ifaces.size(); ++i) {
        if (ifaces.at(i) == QLatin1String("lo"))
            continue;
        ++real;
        rx += readTrimmed(QString("/sys/class/net/%1/statistics/rx_bytes")
                          .arg(ifaces.at(i))).toLongLong();
        tx += readTrimmed(QString("/sys/class/net/%1/statistics/tx_bytes")
                          .arg(ifaces.at(i))).toLongLong();
    }
    set(QLatin1String("net.ifCount"), real);
    if (m_prevRx >= 0 && real > 0) {
        set(QLatin1String("net.rxKbs"), double(rx - m_prevRx) / 1024.0 / 2.0);
        set(QLatin1String("net.txKbs"), double(tx - m_prevTx) / 1024.0 / 2.0);
    }
    m_prevRx = rx;
    m_prevTx = tx;
}

void SystemProbe::refreshOfono()
{
    // ofono's Manager.GetModems returns an array of object paths; reading it
    // means QDBusArgument demarshalling, which we avoid. Sailfish modems are
    // addressed at the fixed /ril_* paths, so probe those directly.
    static const char *const modems[] = { "/ril_0", "/ril_1" };

    QString found;
    QVariantMap reg;
    for (int i = 0; i < 2; ++i) {
        reg = dbusProps(QLatin1String("org.ofono"), QLatin1String(modems[i]),
                        QLatin1String("org.ofono.NetworkRegistration"));
        if (!reg.isEmpty()) {
            found = QLatin1String(modems[i]);
            break;
        }
    }

    if (found.isEmpty()) {
        // No modem, or none of them has registered: the emulator case.
        set(QLatin1String("cell.present"), false);
        unset(QLatin1String("cell.modem"));
        unset(QLatin1String("cell.operator"));
        unset(QLatin1String("cell.tech"));
        unset(QLatin1String("cell.strength"));
        unset(QLatin1String("cell.roaming"));
        unset(QLatin1String("cell.location"));
        unset(QLatin1String("cell.mccmnc"));
        return;
    }

    set(QLatin1String("cell.present"), true);
    set(QLatin1String("cell.modem"), found);

    const QString name = prop(reg, QLatin1String("Name"));
    if (!name.isEmpty())
        set(QLatin1String("cell.operator"), name);
    const QString tech = prop(reg, QLatin1String("Technology"));
    if (!tech.isEmpty())
        set(QLatin1String("cell.tech"), tech);

    const QString area = prop(reg, QLatin1String("LocationAreaCode"));
    const QString cid = prop(reg, QLatin1String("CellId"));
    if (!area.isEmpty() || !cid.isEmpty())
        set(QLatin1String("cell.location"), area + QLatin1String(" / ") + cid);

    const QString mcc = prop(reg, QLatin1String("MobileCountryCode"));
    const QString mnc = prop(reg, QLatin1String("MobileNetworkCode"));
    if (!mcc.isEmpty() || !mnc.isEmpty())
        set(QLatin1String("cell.mccmnc"), mcc + mnc);

    if (reg.contains(QLatin1String("Strength")))
        set(QLatin1String("cell.strength"), reg.value(QLatin1String("Strength")).toInt());
    else
        unset(QLatin1String("cell.strength"));
    if (reg.contains(QLatin1String("Roaming")))
        set(QLatin1String("cell.roaming"), reg.value(QLatin1String("Roaming")).toBool());
    else
        unset(QLatin1String("cell.roaming"));

    const QVariantMap sim = dbusProps(QLatin1String("org.ofono"), found,
                                       QLatin1String("org.ofono.SimManager"));
    if (!sim.isEmpty()) {
        set(QLatin1String("cell.sim"), sim.value(QLatin1String("Present")).toBool());
        set(QLatin1String("cell.simState"), prop(sim, QLatin1String("PinRequired")));
    }
}

void SystemProbe::refreshBluez()
{
    // Only presence and the first adapter: enumerating Device1 objects needs
    // ObjectManager demarshalling, deferred to the phone bring-up.
    QDBusConnectionInterface *busIface = QDBusConnection::systemBus().interface();
    if (!busIface
            || !busIface->isServiceRegistered(QLatin1String("org.bluez")).value()) {
        unset(QLatin1String("bt.present"));
        return;
    }
    set(QLatin1String("bt.present"), true);

    const QVariantMap adapter = dbusGetAll(QLatin1String("org.bluez"),
                                           QLatin1String("/org/bluez/hci0"),
                                           QLatin1String("org.bluez.Adapter1"));
    if (adapter.isEmpty()) {
        unset(QLatin1String("bt.powered"));
        unset(QLatin1String("bt.discovering"));
        return;
    }
    set(QLatin1String("bt.powered"), adapter.value(QLatin1String("Powered")).toBool());
    set(QLatin1String("bt.discovering"), adapter.value(QLatin1String("Discovering")).toBool());
    set(QLatin1String("bt.address"), prop(adapter, QLatin1String("Address")));
    set(QLatin1String("bt.alias"), prop(adapter, QLatin1String("Alias")));
}

// ---------------------------------------------------------------------- mic

void SystemProbe::setMicActive(bool active)
{
    if (active == m_micActive)
        return;

    if (active) {
        const QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
        if (info.isNull()) {
            set(QLatin1String("mic.supported"), false);
            if (m_dirty) { m_dirty = false; emit valuesChanged(); }
            return;
        }

        QAudioFormat format = info.preferredFormat();
        format.setSampleType(QAudioFormat::SignedInt);
        format.setSampleSize(16);
        if (!info.isFormatSupported(format))
            format = info.nearestFormat(format);

        if (format.sampleSize() != 16 || format.sampleType() != QAudioFormat::SignedInt) {
            set(QLatin1String("mic.supported"), false);
            if (m_dirty) { m_dirty = false; emit valuesChanged(); }
            return;
        }

        m_audio = new QAudioInput(info, format, this);
        m_audioDev = m_audio->start();
        if (!m_audioDev) {
            delete m_audio;
            m_audio = 0;
            set(QLatin1String("mic.supported"), false);
            if (m_dirty) { m_dirty = false; emit valuesChanged(); }
            return;
        }
        connect(m_audioDev, SIGNAL(readyRead()), this, SLOT(onMicReadyRead()));
        set(QLatin1String("mic.supported"), true);
        m_micActive = true;
        emit micActiveChanged();
    } else {
        if (m_audio) {
            m_audio->stop();
            delete m_audio;
            m_audio = 0;
            m_audioDev = 0;
        }
        set(QLatin1String("mic.level"), 0.0);
        set(QLatin1String("mic.peak"), 0.0);
        m_micActive = false;
        emit micActiveChanged();
    }

    if (m_dirty) {
        m_dirty = false;
        emit valuesChanged();
    }
}

void SystemProbe::onMicReadyRead()
{
    if (!m_audioDev)
        return;

    const QByteArray data = m_audioDev->readAll();
    if (data.size() < 2)
        return;

    const int samples = data.size() / 2;
    const qint16 *pcm = reinterpret_cast<const qint16 *>(data.constData());
    double sumSquares = 0.0;
    int peak = 0;
    for (int i = 0; i < samples; ++i) {
        const int v = pcm[i];
        const int a = v < 0 ? -v : v;
        if (a > peak)
            peak = a;
        sumSquares += double(v) * double(v);
    }

    const double rms = qSqrt(sumSquares / double(samples)) / 32768.0;
    const double peakLevel = double(peak) / 32768.0;

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastMicEmit < 120)
        return;
    m_lastMicEmit = now;

    set(QLatin1String("mic.level"), rms);
    set(QLatin1String("mic.peak"), peakLevel);
    if (m_dirty) {
        m_dirty = false;
        emit valuesChanged();
    }
}
