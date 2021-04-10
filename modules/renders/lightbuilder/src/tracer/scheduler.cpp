#include "tracer/scheduler.h"

#include <QDebug>
#include <QFile>

#include <components/camera.h>
#include <components/baselight.h>

#include "tracer/worker.h"

#include "components/alightsourcert.h"

Scheduler::Scheduler() {
    mBaking  = false;
    pHdr     = nullptr;

    mCameraTarget      = true;
    mCameraDOF         = false;
    mCameraMOB         = false;
    mCameraFNumber     = 1.0f;
    mCameraFocalLength = 1.0f;
    mCameraSPP         = 1;

    mFirstBounce  = BACK_TRACING;
    mSecondBounce = BACK_TRACING;

    mPathTracingSPP        = 1;
    mPathTracingMaxBounces = 1;

    mPhotonsSamples        = 0;
    mPhotonsDiffuse        = 0;
    mPhotonsCaustic        = 0;
    mPhotonsCollectAuto    = false;
    mPhotonsCollectRadius  = 0.0f;
    mPhotonsCollectCount   = 0;

    mWidth      = 1;
    mHeight     = 1;

    mCellWidth  = 32;
    mCellHeight = 32;
    mMaxThreads = 1;

    bHardware   = false;

    QStringList list;
    list << "utils.cl" << "ray.cl" << "kernel.cl";

    QByteArray source;
    foreach (QString str, list) {
        QFile file(QString(":/programs/") + str);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            source.append(file.readAll());
            file.close();
        } else {
            qDebug() << "Can't include " << file.fileName();
        }
    }

    oclStart(source.toStdString());
}

Scheduler::~Scheduler() {
    delete []pHdr;
}

void Scheduler::oclStart(const string &source) {
    const char *data = source.c_str();
    uint32_t length = source.size();
/*
    cl_int error = CL_SUCCESS;
    cl_uint numPlatforms;
    if(clGetPlatformIDs(0, nullptr, &numPlatforms) == CL_SUCCESS) {

        cl_platform_id *platforms = new cl_platform_id[numPlatforms];
        if(clGetPlatformIDs(numPlatforms, platforms, nullptr) == CL_SUCCESS) {

            for(cl_uint p = 0; p < numPlatforms; p++) {
                cl_uint numDevices;
                if(clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, 0, nullptr, &numDevices) == CL_SUCCESS) {

                    cl_device_id device;
                    if(clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, numDevices, &device, nullptr) == CL_SUCCESS) {
                        cl_context context = clCreateContext(0, numDevices, &device, nullptr, nullptr, &error);
                        if(error == CL_SUCCESS) {
                            cl_program program = clCreateProgramWithSource(context, 1, &data, &length, &error);
                            if(error == CL_SUCCESS) {
                                if(clBuildProgram(program, 0, nullptr, "", nullptr, nullptr) != CL_SUCCESS) {
                                    error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &length);
                                    if(error == CL_SUCCESS) {
                                        char *log = new char[length];
                                        error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, length, log, nullptr);
										delete[]log;

										clReleaseProgram(program);
										clReleaseContext(context);
									}
                                    return;
                                } else {
                                    mDevices.push_back(device);
                                    mContexts.push_back(context);
                                    mPrograms.push_back(program);
                                }
                            }
                            bHardware = true;
                        }
                    }
                }
            }
        }
    }
*/
}

void Scheduler::oclStop() {
/*
    for(int c = 0; c < mContexts.size(); c++) {
        clReleaseContext(mContexts[c]);
    }

    for(int p = 0; p < mPrograms.size(); p++) {
        clReleaseProgram(mPrograms[p]);
    }
*/
}

void Scheduler::oclDevices(vector<string> &list) {
/*
    for(uint32_t i = 0; i < mDevices.size(); i++) {
        size_t length;
        clGetDeviceInfo(mDevices[i], CL_DEVICE_NAME, 0, nullptr, &length);

        char *name          = new char[length];
        clGetDeviceInfo(mDevices[i], CL_DEVICE_NAME, length, name, nullptr);
        list.push_back(name);
        delete []name;
    }
*/
}

Vector4 *Scheduler::create(uint32_t w, uint32_t h, bool reverse, bool baking) {
    delete []pHdr;

    mWidth  = w;
    mHeight = h;

    Camera *camera = nullptr;

    pHdr    = new Vector4[mWidth * mHeight];
    mBaking = baking;

    if(mFirstBounce == PHOTON_MAPPING || mSecondBounce == PHOTON_MAPPING) {
        //emitPhotons();
        //
        //m_pScene->mPhotonTree.i     = m_pScene->mPhotons;
        //m_pScene->mPhotonTree.left  = 0;
        //m_pScene->mPhotonTree.right = 0;
        //
        //buildPhotonsTree(&m_pScene->mPhotonTree, 0);
        //qDebug() << "Photon tree has been constructed.";
    }

    mTasks.clear();

    uint32_t x  = 0;
    uint32_t y  = 0;

    uint32_t cw = 0;
    uint32_t ch = 0;

    while(true) {
        if((x + cw) > (mWidth - mCellWidth)) {
            cw = mWidth - x;
        } else {
            cw = mCellWidth;
        }

        if((y + ch) > (mHeight - mCellHeight)) {
            ch = mHeight - y;
        } else {
            ch = mCellHeight;
        }
        TaskData task;
        task.posX   = x;
        task.posY   = x;
        task.sizeX  = cw;
        task.sizeY  = ch;
        task.state  = TASK_WAIT;
        task.device = -1;

        task.processed  = 0;

        if(reverse) {
            mTasks.push_front(task);
        } else {
            mTasks.push_back(task);
        }

        x += cw;
        if(x > (mWidth - mCellWidth)) {
            x  = 0;
            y += ch;
            if(y > (mHeight - mCellHeight)) {
                break;
            }
        }
    }
    qDebug() << mTasks.size() << "Tasks added.";

    return pHdr;
}

void Scheduler::start() {
    restart();
}

void Scheduler::stop() {
    for(auto &it : mTasks) {
        it.state = TASK_DROP;
    }

    emit allStop();
}

void Scheduler::restart() {
    mTime.start();

    memset(pHdr, 0, sizeof(Vector3) * mWidth * mHeight);

    for(auto &it : mTasks) {
        it.state = TASK_WAIT;
    }
    if(!cDevices.empty()) {
        for(uint32_t t = 0; t < cDevices.size(); t++) {
            //onDone(cDevices[t]);
        }
    } else {
        for(int t = 0; t < mMaxThreads; t++) {
            onDone(-1);
        }
    }
}

void Scheduler::onDone(int device) {
    // Get task
    for(auto &it : mTasks) {
        switch(it.state) {
            case TASK_WAIT: {
                addThread(&it, device);
                return;
            } break;
            default: break;
        }
    }

    qDebug("All done. Time elapsed: %0.1f sec.", mTime.elapsed() / 1000.0);
    emit allDone();
}

void Scheduler::onUpdateResult(const QRect &r) {
    float percent   = 0.0f;
    for(auto &it : mTasks) {
        percent += static_cast<float>(it.processed) / (static_cast<float>(mPathTracingSPP) * 0.01f);
    }
    percent /= static_cast<float>(mTasks.size()) * 100;

    emit updateProgress(percent, mTime.elapsed());
    emit updateResult(r);
}

void Scheduler::addThread(TaskData *task, int device) {
    task->state     = TASK_PROGRESS;
    task->device    = device;

    Worker *worker  = new Worker(this, task);

    QThread *thread = new QThread(this);
    worker->moveToThread(thread);

    connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    connect(worker, SIGNAL(updated(QRect)), this, SLOT(onUpdateResult(QRect)));
    connect(worker, SIGNAL(finished(int)), this, SLOT(onDone(int)));
    connect(worker, SIGNAL(finished(int)), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished(int)), worker, SLOT(deleteLater()));

    thread->start();
}

void Scheduler::setScene(int mode) {
    mTime.start();

    //m_pScene->compose(mode);

    qDebug("Compose stage complete. Time elapsed: %0.1f sec.", mTime.elapsed() / 1000.0);
    mTime.restart();
}

void Scheduler::setCameraSettings(float f, bool target, float focal, bool dof, bool mob, int subdiv) {
    mCameraTarget       = target;
    mCameraDOF          = dof;
    mCameraMOB          = mob;
    mCameraFNumber      = f;
    mCameraFocalLength  = focal;
    mCameraSPP          = subdiv * subdiv;
}

void Scheduler::setCellSize(int width, int height) {
    mCellWidth = width;
    mCellHeight = height;
}

void Scheduler::setMaxThreads(int value) {
    mMaxThreads = value;
}

void Scheduler::setBounceEngines(CollectTypes first, CollectTypes second) {
    mFirstBounce  = first;
    mSecondBounce = second;
}

void Scheduler::setPathTracing(int subdiv, int bounce) {
    mPathTracingSPP        = subdiv * subdiv;
    mPathTracingMaxBounces = bounce;
}

void Scheduler::setPhotonMapping(int samples, int diffuse, int caustic, bool flag, float radius, int count, int seed, int depth) {
    mPhotonsSamples       = samples;
    mPhotonsDiffuse       = diffuse;
    mPhotonsCaustic       = caustic;
    mPhotonsCollectAuto   = flag;
    mPhotonsCollectRadius = radius;
    mPhotonsCollectCount  = count;
    mPhotonsRandomSeed    = seed;
    mPhotonsThreeDepth    = depth * 3;
}

void Scheduler::addDevice(int device) {
    if(device > -1)
        cDevices.push_back(device);
    else
        cDevices.clear();
}

