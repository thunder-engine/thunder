#include "os/filedialog.h"

#include <string>
#include <vector>
#include <filesystem>
#include <functional>
#include <memory>

namespace fs = std::filesystem;

class FileDialogPrivate {
public:
    FileDialogPrivate() :
        m_mode(FileDialog::OpenFile),
        m_showHidden(false) {

    }

    ~FileDialogPrivate() = default;

    bool exec();

private:
    friend class FileDialog;

#ifdef _WIN32
    bool execWindows();
    bool execWindowsFile();
    bool execWindowsDirectory();
#endif

#ifdef __APPLE__
    bool execMacOS();
#endif

#ifdef __linux__
    bool execLinux();
#endif

    struct Filter {
        TString name;
        StringList extensions;
    };

    std::vector<Filter> m_filters;
    TString m_defaultSuffix;
    TString m_windowTitle;
    TString m_initialDir;
    StringList m_selectedFiles;

    FileDialog::Mode m_mode;

    bool m_showHidden;

};

bool FileDialogPrivate::exec() {
    m_selectedFiles.clear();

#ifdef _WIN32
    return execWindows();
#elif __APPLE__
    return execMacOS();
#elif __linux__
    return execLinux();
#else
#error "Unsupported platform"
#endif
}

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Ole32.lib")

bool FileDialogPrivate::execWindows() {
    if(m_mode == FileDialog::OpenDirectory) {
        return execWindowsDirectory();
    }
    return execWindowsFile();
}

bool FileDialogPrivate::execWindowsFile() {
    std::string filterStr;
    for(const auto &f : m_filters) {
        filterStr += f.name.toStdString() + '\0';

        std::string patterns;
        for(const auto& ext : f.extensions) {
            if (!patterns.empty()) patterns += ";";
            patterns += "*" + ext.toStdString();
        }
        if(patterns.empty()) patterns = "*.*";
        filterStr += patterns + '\0';
    }
    if(m_filters.empty()) {
        filterStr = "All Files\0*.*\0";
    }

    OPENFILENAMEA ofn = {};
    char fileName[MAX_PATH] = {};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filterStr.c_str();
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = m_initialDir.isEmpty() ? nullptr : m_initialDir.toStdString().c_str();
    ofn.lpstrTitle = m_windowTitle.isEmpty() ? nullptr : m_windowTitle.toStdString().c_str();

    DWORD flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if(m_mode == FileDialog::SaveFile) {
        flags |= OFN_OVERWRITEPROMPT;
    } else {
        flags |= OFN_FILEMUSTEXIST;
    }

    if(m_mode == FileDialog::OpenFiles) {
        flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
    }

    ofn.Flags = flags;

    BOOL result;
    if(m_mode == FileDialog::SaveFile) {
        result = GetSaveFileNameA(&ofn);
    } else {
        result = GetOpenFileNameA(&ofn);
    }

    if(!result) {
        DWORD err = CommDlgExtendedError();
        if(err != 0) {

        }
        return false;
    }

    if(m_mode == FileDialog::OpenFiles && (ofn.Flags & OFN_ALLOWMULTISELECT)) {
        char* ptr = fileName;
        fs::path baseDir(ptr);
        ptr += strlen(ptr) + 1;

        if(*ptr == '\0') {
            m_selectedFiles.push_back(TString(baseDir.string()));
        } else {
            while (*ptr != '\0') {
                m_selectedFiles.push_back(TString((baseDir / fs::path(ptr)).string()));
                ptr += strlen(ptr) + 1;
            }
        }
    } else {
        m_selectedFiles.push_back(TString(fileName));
    }

    return true;
}

bool FileDialogPrivate::execWindowsDirectory() {
    BROWSEINFOA bi = {};
    char path[MAX_PATH] = {};

    bi.hwndOwner = GetActiveWindow();
    bi.pszDisplayName = path;
    bi.lpszTitle = m_windowTitle.isEmpty() ? "Select Directory" : m_windowTitle.toStdString().c_str();
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn = nullptr;

    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl != nullptr) {
        if (SHGetPathFromIDListA(pidl, path)) {
            m_selectedFiles.push_back(TString(path));
            CoTaskMemFree(pidl);
            return true;
        }
        CoTaskMemFree(pidl);
    }
    return false;
}
#endif // _WIN32

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>
#include <objc/objc.h>
#include <objc/objc-runtime.h>

bool FileDialogPrivate::execMacOS() {
    Class panelClass;
    id panel;

    if(m_mode == FileDialog::SaveFile) {
        panelClass = objc_getClass("NSSavePanel");
        panel = objc_msgSend(panelClass, sel_getUid("savePanel"));
    } else if(m_mode == FileDialog::OpenDirectory) {
        panelClass = objc_getClass("NSOpenPanel");
        panel = objc_msgSend(panelClass, sel_getUid("openPanel"));
        objc_msgSend(panel, sel_getUid("setCanChooseDirectories:"), YES);
        objc_msgSend(panel, sel_getUid("setCanChooseFiles:"), NO);
    } else {
        panelClass = objc_getClass("NSOpenPanel");
        panel = objc_msgSend(panelClass, sel_getUid("openPanel"));
        objc_msgSend(panel, sel_getUid("setCanChooseDirectories:"), NO);
        objc_msgSend(panel, sel_getUid("setCanChooseFiles:"), YES);

        if(m_mode == FileDialog::OpenFiles) {
            objc_msgSend(panel, sel_getUid("setAllowsMultipleSelection:"), YES);
        }
    }

    if(!m_windowTitle.isEmpty()) {
        NSString *title = objc_msgSend(objc_getClass("NSString"), sel_getUid("stringWithUTF8String:"), m_windowTitle.data());
        objc_msgSend(panel, sel_getUid("setTitle:"), title);
    }

    if(!m_initialDir.isEmpty()) {
        NSString* path = objc_msgSend(objc_getClass("NSString"),
                                      sel_getUid("stringWithUTF8String:"), m_initialDir.data());
        NSURL* url = objc_msgSend(objc_getClass("NSURL"),
                                  sel_getUid("fileURLWithPath:"), path);
        objc_msgSend(panel, sel_getUid("setDirectoryURL:"), url);
    }

    if(!m_filters.isEmpty()) {
        NSMutableArray* allowedTypes = objc_msgSend(objc_getClass("NSMutableArray"), sel_getUid("array"));

        for(const auto& filter : m_filters) {
            for(const auto& ext : filter.extensions) {
                if(!ext.empty()) {
                    std::string extStr = ext.toStdString();
                    if(!extStr.empty() && extStr[0] == '.') {
                        extStr = extStr.substr(1);
                    }
                    NSString* extNSString = objc_msgSend(objc_getClass("NSString"),
                                                         sel_getUid("stringWithUTF8String:"), extStr.c_str());
                    objc_msgSend(allowedTypes, sel_getUid("addObject:"), extNSString);
                }
            }
        }

        if(objc_msgSend(allowedTypes, sel_getUid("count")) > 0) {
            objc_msgSend(panel, sel_getUid("setAllowedFileTypes:"), allowedTypes);
        }
    }

    NSInteger result = (NSInteger)objc_msgSend(panel, sel_getUid("runModal"));

    if(result == NSModalResponseOK) {
        id urls = objc_msgSend(panel, sel_getUid("URLs"));
        NSUInteger count = (NSUInteger)objc_msgSend(urls, sel_getUid("count"));

        for(NSUInteger i = 0; i < count; i++) {
            id url = objc_msgSend(urls, sel_getUid("objectAtIndex:"), i);
            id path = objc_msgSend(url, sel_getUid("path"));
            const char* cpath = objc_msgSend(path, sel_getUid("UTF8String"));
            selectedFiles.push_back(TString(cpath));
        }
        return true;
    }

    return false;
}
#endif // __APPLE__

#ifdef __linux__
#include <gtk/gtk.h>
#include <unistd.h>
#include <cstdlib>

bool FileDialogPrivate::execLinux() {
    static bool gtkInitialized = false;
    if(!gtkInitialized) {
        gtk_init(nullptr, nullptr);
        gtkInitialized = true;
    }

    GtkWidget *dialog;
    GtkFileChooserAction action;
    const char *buttonText;

    switch(m_mode) {
    case FileDialog::SaveFile:
        action = GTK_FILE_CHOOSER_ACTION_SAVE;
        buttonText = "_Save";
        break;
    case FileDialog::OpenDirectory:
        action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
        buttonText = "_Open";
        break;
    default:
        action = GTK_FILE_CHOOSER_ACTION_OPEN;
        buttonText = "_Open";
        break;
    }

    dialog = gtk_file_chooser_dialog_new(
        m_windowTitle.isEmpty() ? nullptr : m_windowTitle.data(),
        nullptr,
        action,
        "_Cancel", GTK_RESPONSE_CANCEL,
        buttonText, GTK_RESPONSE_ACCEPT,
        nullptr
        );

    if(m_mode == FileDialog::OpenFiles) {
        gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
    }

    if(!m_initialDir.isEmpty()) {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), m_initialDir.data());
    }

    GtkFileFilter *allFilter = nullptr;
    for(const auto &filter : m_filters) {
        GtkFileFilter* gtkFilter = gtk_file_filter_new();
        gtk_file_filter_set_name(gtkFilter, filter.name.data());

        for(const auto &ext : filter.extensions) {
            std::string pattern = "*" + ext.toStdString();
            gtk_file_filter_add_pattern(gtkFilter, pattern.c_str());
        }

        if(filter.extensions.empty()) {
            gtk_file_filter_add_pattern(gtkFilter, "*");
        }

        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), gtkFilter);

        if(filter.name == "All Files" || filter.extensions.empty()) {
            allFilter = gtkFilter;
        }
    }

    if(m_filters.empty()) {
        GtkFileFilter *gtkFilter = gtk_file_filter_new();
        gtk_file_filter_set_name(gtkFilter, "All Files");
        gtk_file_filter_add_pattern(gtkFilter, "*");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), gtkFilter);
        allFilter = gtkFilter;
    }

    if(allFilter) {
        gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), allFilter);
    }

    int response = gtk_dialog_run(GTK_DIALOG(dialog));

    if(response == GTK_RESPONSE_ACCEPT) {
        GSList *filenames = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));

        for(GSList *iter = filenames; iter != nullptr; iter = iter->next) {
            char *filename = (char*)iter->data;
            m_selectedFiles.push_back(TString(filename));
            g_free(filename);
        }

        g_slist_free(filenames);
        gtk_widget_destroy(dialog);
        return true;
    }

    gtk_widget_destroy(dialog);
    return false;
}
#endif // __linux__

FileDialog::FileDialog() :
    m_ptr(new FileDialogPrivate) {

}

FileDialog::~FileDialog() {
    delete m_ptr;
}

FileDialog::FileDialog(FileDialog&&) noexcept = default;
FileDialog& FileDialog::operator=(FileDialog&&) noexcept = default;

void FileDialog::setMode(Mode m) {
    m_ptr->m_mode = m;
}

void FileDialog::setShowHidden(bool show) {
    m_ptr->m_showHidden = show;
}

void FileDialog::setDefaultSuffix(const TString &suffix) {
    m_ptr->m_defaultSuffix = suffix;
}

void FileDialog::setWindowTitle(const TString &title) {
    m_ptr->m_windowTitle = title;
}

void FileDialog::addFilter(const TString &name, const StringList &extensions) {
    m_ptr->m_filters.push_back({name, extensions});
}

void FileDialog::clearFilters() {
    m_ptr->m_filters.clear();
}

void FileDialog::setDirectory(const TString &dir) {
    if(fs::exists(dir.toStdString()) && fs::is_directory(dir.toStdString())) {
        m_ptr->m_initialDir = dir;
    }
}

TString FileDialog::getSelectedFile() const {
    return m_ptr->m_selectedFiles.empty() ? TString() : m_ptr->m_selectedFiles.front();
}

StringList FileDialog::getSelectedFiles() const {
    return m_ptr->m_selectedFiles;
}

bool FileDialog::exec() {
    return m_ptr->exec();
}
