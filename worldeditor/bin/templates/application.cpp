#include <engine.h>

#include <file.h>
#include <log.h>
#include <platform.h>

#include <rendergl.h>

class SimpleHandler : public ILogHandler {
protected:
    void            setRecord       (Log::LogTypes type, const char *record) {
        FILE *fp    = fopen("log.txt", "a");
        if(fp) {
            fwrite(record, strlen(record), 1, fp);
            fclose(fp);
        }
    }
};

int main(int argc, char **argv) {
    Log::overrideHandler(new SimpleHandler());
    Log::setLogLevel(Log::DBG);

    IFile *file = new IFile;
    file->finit(argv[0]);
    file->fsearchPathAdd("base.pak");

    Engine engine(file);
    engine.addModule(new RenderGL(&engine));
    if (engine.init() && engine.createWindow()) {
        engine.exec();
    }
    return 0;
}

THUNDER_MAIN()
