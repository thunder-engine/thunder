#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QTime>
#include <QThread>
#include <QRect>

#include <object.h>

enum CollectTypes {
    BACK_TRACING    = 1,
    PHOTON_MAPPING  = 2,
    PATH_TRACING    = 3
};

enum TaskStateTypes {
    TASK_WAIT       = 1,
    TASK_PROGRESS,
    TASK_DONE,
    TASK_DROP
};

struct TaskData {
    TaskStateTypes state;

    int device;
    int processed;

    uint32_t posX;
    uint32_t posY;

    uint32_t sizeX;
    uint32_t sizeY;
};

typedef list<TaskData>   TaskList;
typedef vector<uint32_t> ActiveVector;

class Scheduler : public QObject {
    Q_OBJECT

public:
    Scheduler     ();
    ~Scheduler    ();

    Vector4      *create                (uint32_t w, uint32_t h, bool reverse, bool baking = false);
    void          start                 ();
    void          stop                  ();
    void          restart               ();

    void          oclStart              (const string &source);
    void          oclStop               ();
    void          oclDevices            (vector<string> &list);

    uint32_t      width                 () { return mWidth; }
    uint32_t      height                () { return mHeight; }

    bool          baking                () { return mBaking; }

    Vector4      *hdr                   () { return pHdr; }

    CollectTypes  firstBounce           () { return mFirstBounce; }
    CollectTypes  secondBounce          () { return mSecondBounce; }

    int           spp                   () { return (mFirstBounce == PATH_TRACING) ? mPathTracingSPP : 1; }
    int           diffuse               () { return (mFirstBounce == PATH_TRACING) ? mPathTracingMaxBounces : 1; }

    float         cameraFNumber         () { return mCameraFNumber; }
    float         cameraFocal           () { return mCameraFocalLength; }
    int           cameraSPP             () { return mCameraSPP; }
    bool          cameraTarget          () { return mCameraTarget; }
    bool          cameraDOF             () { return mCameraDOF; }

    int           pathTracingMaxBounces () { return mPathTracingMaxBounces; }

    int           photonSamplesPhotons  () { return mPhotonsSamples; }
    float         photonCollectRadius   () { return mPhotonsCollectRadius; }
    int           photonCollectCount    () { return mPhotonsCollectCount; }
    bool          photonCollectAuto     () { return mPhotonsCollectAuto; }



    void          setScene              (int mode);

    void          setCameraSettings     (float f, bool target, float focal, bool dof, bool mob, int subdiv);

    void          setCellSize           (int width, int height);
    void          setMaxThreads         (int value);

    void          setBounceEngines      (CollectTypes first, CollectTypes second);

    void          setPathTracing        (int subdiv, int bounce);
    void          setPhotonMapping      (int samples, int diffuse, int caustic, bool flag, float radius, int count, int seed, int depth);
    void          addDevice             (int device);

    void          addThread             (TaskData *task, int device);

    Vector3       texureColor           (const Vector3 &point, Vector3 &v1, Vector3 &v2, Vector3 &v3, Vector2 &t1, Vector2 &t2, Vector2 &t3);

    bool          isHardware            () { return bHardware; }

public slots:
    void          onDone                (int device);
    void          onUpdateResult        (const QRect &r);

signals:
    void          allDone               ();
    void          allStop               ();

    void          updateResult          (QRect r);
    void          updateProgress        (float percent, int elapsed);

private:
    bool          bHardware;
    ActiveVector  cDevices;

    QTime         mTime;

    TaskList      mTasks;

    Vector4      *pHdr;

    bool          mCameraTarget;
    bool          mCameraDOF;
    bool          mCameraMOB;
    float         mCameraFNumber;
    float         mCameraFocalLength;
    int           mCameraSPP;

    CollectTypes  mFirstBounce;
    CollectTypes  mSecondBounce;

    int           mPathTracingSPP;
    int           mPathTracingMaxBounces;

    int           mPhotonsSamples;
    int           mPhotonsDiffuse;
    int           mPhotonsCaustic;
    bool          mPhotonsCollectAuto;
    float         mPhotonsCollectRadius;
    int           mPhotonsCollectCount;
    int           mPhotonsRandomSeed;
    int           mPhotonsThreeDepth;

    uint32_t      mCellWidth;
    uint32_t      mCellHeight;
    int           mMaxThreads;

    uint32_t      mWidth;
    uint32_t      mHeight;

    bool          mBaking;
};
#endif // SCHEDULER_H
