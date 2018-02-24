#include <engine.h>

#include <file.h>
#include <log.h>
#include <platform.h>

#include <rendergl.h>

#include <mutex>

static IFile *file  = nullptr;

class SimpleHandler : public ILogHandler {
protected:
    void            setRecord       (Log::LogTypes, const char *record) {
        unique_lock<mutex> locker(m_Mutex);
        _FILE *fp   = file->_fopen("log.txt", "a");
        if(fp) {
            file->_fwrite(record, strlen(record), 1, fp);
            file->_fclose(fp);
        }
    }
    mutex           m_Mutex;
};

int main(int argc, char **argv) {
    Log::overrideHandler(new SimpleHandler());
    Log::setLogLevel(Log::ERR);

    file    = new IFile;
    file->finit(argv[0]);
    Engine engine(file, argc, argv);

    file->fsearchPathAdd(engine.locationConfigDir().c_str(), true);
    file->fsearchPathAdd((engine.locationAppDir() + "/base.pak").c_str());

    engine.addModule(new RenderGL(&engine));
    if(engine.init() && engine.createWindow()) {
        engine.exec();
    }
    return 0;
}

THUNDER_MAIN()
