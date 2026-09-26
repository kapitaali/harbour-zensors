#ifndef SYSTEMPROBE_H
#define SYSTEMPROBE_H

#include <QObject>
#include <QVariantMap>
#include <QStringList>
#include <QVector>
#include <QDateTime>
#include <QHash>
#include <QSet>

class QAudioInput;
class QIODevice;
class QTimer;
class QQuickWindow;

// Opaque GLib handles: gio/gio.h is included by the .cpp only, these are the
// two pointer types the async D-Bus reply callback signature needs.
typedef struct _GObject GObject;
typedef struct _GAsyncResult GAsyncResult;

/*
 * SystemProbe: reads every meter the sandbox lets us read and publishes the
 * result as a flat QVariantMap so QML can bind to it:
 *
 *     probe.values["cpu.total"]  ->  17.5
 *
 * Two groups of readers:
 *   - file based (/proc, /sys) every tick (2 s)
 *   - D-Bus based (MCE, connman, ofono) every 4th tick (8 s)
 *
 * Everything is optional by construction: a missing file, a filtered D-Bus
 * service or an absent device simply leaves the key unset, which QML renders
 * as "not available on this device".
 */
class SystemProbe : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap values READ values NOTIFY valuesChanged)
    Q_PROPERTY(bool micActive READ micActive NOTIFY micActiveChanged)

public:
    // window is optional; when set, SIGUSR1 dumps it to /tmp/app-shot.png
    explicit SystemProbe(QObject *parent = 0);
    void setWindow(QQuickWindow *window) { m_window = window; }

    QVariantMap values() const { return m_values; }
    bool micActive() const { return m_micActive; }

    Q_INVOKABLE QString readFile(const QString &path) const;
    Q_INVOKABLE QStringList listDir(const QString &path) const;
    Q_INVOKABLE void refresh();

public slots:
    void setMicActive(bool active);

signals:
    void valuesChanged();
    void micActiveChanged();

private slots:
    void tick();
    void onMicReadyRead();

private:
    void set(const QString &key, const QVariant &value);
    void unset(const QString &key);

    void refreshCpu();
    void refreshMemory();
    void refreshStorage();
    void refreshPower();
    void refreshThermal();
    void refreshDisplay();
    void refreshSystem();

    void refreshDbus();
    void refreshConnman();
    void refreshOfono();
    void refreshMce();
    void refreshBluez();

    // Property maps are read through GDBus (libgio) asynchronously: Qt's own
    // demarshaller for a{sv} replies hands back empty keys here, and reading
    // them as QVariant crashes inside libdbus. The reply is printed as GVariant
    // text - the same text "gdbus call" produced - and scanned by
    // parseGdbusMap(). Results land in m_mapCache, so the two getters below
    // are plain lookups.
    void startMapCall(const QString &service, const QString &path,
                      const QString &iface, const QString &method,
                      const QStringList &args = QStringList());
    QVariantMap dbusProps(const QString &service, const QString &path,
                          const QString &iface) const;   // iface.GetProperties
    QVariantMap dbusGetAll(const QString &service, const QString &path,
                           const QString &iface) const;  // Properties.GetAll
    QVariant dbusCall(const QString &service, const QString &path,
                      const QString &iface, const QString &method) const;
    static QString prop(const QVariantMap &m, const QString &key);

    // GDBus delivers its replies through a GLib main context, and Qt's event
    // loop does not run one; pumpGlib() iterates it while calls are in flight
    // (started lazily by startGlibPump(), stopped when the last reply lands).
    static void mapCallThunk(GObject *source, GAsyncResult *result,
                             void *userData);
    void startGlibPump();
    void pumpGlib();

    QVariantMap m_values;
    bool m_dirty;
    int m_ticks;
    QDateTime m_lastTick;
    QQuickWindow *m_window;

    // async GDBus property-map reads
    QHash<QString, QVariantMap> m_mapCache;
    QSet<QString> m_pendingKeys;
    QTimer *m_glibPump;

    qint64 m_prevCpuTotal;
    qint64 m_prevCpuIdle;
    QVector<qint64> m_prevCoreTotal;
    QVector<qint64> m_prevCoreIdle;
    qint64 m_prevIoRead;
    qint64 m_prevIoWrite;
    qint64 m_prevRx;
    qint64 m_prevTx;

    QAudioInput *m_audio;
    QIODevice *m_audioDev;
    bool m_micActive;
    qint64 m_lastMicEmit;
};

#endif // SYSTEMPROBE_H
