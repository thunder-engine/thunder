#ifndef STANDARDPATHS_H
#define STANDARDPATHS_H

#include <astring.h>

class NEXT_LIBRARY_EXPORT StandardPaths {
public:
    enum StandardLocation {
        ApplicationsLocation,
        DocumentsLocation,
        CacheLocation,
        AppDataLocation,
        AppLocalDataLocation,
        ConfigLocation,
        DownloadLocation,
        MusicLocation,
        PicturesLocation,
        VideosLocation,
        TempLocation,
        HomeLocation,
        RuntimeLocation
    };

    static TString writableLocation(StandardLocation type);

    static StringList standardLocations(StandardLocation type);

    static TString appendOrganizationAndApp(const TString &base,
                                            const TString &organizationName,
                                            const TString &applicationName);

private:
    StandardPaths() = delete;
};

#endif // STANDARDPATHS_H
