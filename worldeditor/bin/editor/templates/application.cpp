#include <engine.h>

#include <file.h>
#include <log.h>
#include <platform.h>

#include <rendergl.h>

static string sLogPath;

class SimpleHandler : public ILogHandler {
protected:
    void            setRecord       (Log::LogTypes, const char *record) {
        FILE *fp    = fopen(sLogPath.c_str(), "a");
        if(fp) {
            fprintf(fp, "%s\r\n", record);
            fclose(fp);
        }
    }
};

int main(int, char **argv) {
    Log::overrideHandler(new SimpleHandler());
    Log::setLogLevel(Log::ERR);

    IFile *file = new IFile;
    file->finit(argv[0]);
    Engine engine(file);

    sLogPath    = (string(file->userDir()) + "/log.txt");

    file->fsearchPathAdd((string(file->baseDir()) + "/base.pak").c_str());

    engine.addModule(new RenderGL(&engine));
    if(engine.init() && engine.createWindow()) {
        engine.exec();
    }
    return 0;
}

THUNDER_MAIN()
