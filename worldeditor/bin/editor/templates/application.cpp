#include <engine.h>

#include <file.h>
#include <log.h>
#include <platform.h>

#include <rendergl.h>

#include <mutex>
#include <string.h>

${Includes}

static string gAppConfig;
static IFile *gFile = nullptr;

class SimpleHandler : public ILogHandler {
protected:
    void            setRecord       (Log::LogTypes, const char *record) {
        unique_lock<mutex> locker(m_Mutex);
        _FILE *fp   = gFile->_fopen((gAppConfig + "/log.txt").c_str(), "a");
        if(fp) {
            gFile->_fwrite(record, strlen(record), 1, fp);
            gFile->_fwrite("\n", 1, 1, fp);
            gFile->_fclose(fp);
        }
    }
    mutex           m_Mutex;
};

int main(int argc, char **argv) {
    Log::overrideHandler(new SimpleHandler());
    Log::setLogLevel(Log::DBG);

    gFile   = new IFile;
    gFile->finit(argv[0]);
    Engine engine(gFile, argc, argv);
    {
        ObjectSystem system;
        ${RegisterComponents}
    }

    gAppConfig  = engine.locationAppConfig();

    gFile->fsearchPathAdd(engine.locationConfig().c_str(), true);
    gFile->_mkdir(gAppConfig.c_str());

    gFile->fsearchPathAdd((engine.locationAppDir() + "/base.pak").c_str());
    engine.addModule(new RenderGL(&engine));
    if(engine.init() && engine.createWindow()) {
        engine.exec();
    }

    {
        ObjectSystem system;
        ${UnregisterComponents}
    }
    return 0;
}

THUNDER_MAIN()
