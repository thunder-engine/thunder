import qbs

InstallPackage {
    id: archive

    property string architecture: {
        if(qbs.targetOS.contains("darwin") || qbs.targetOS.contains("linux")) {
            return "x86_64"
        }
        return qbs.architecture;
    }

    targetName: "ThunderEngine-" + qbs.targetOS[0] + "-" + archive.architecture

    archiver.type: (qbs.targetOS.contains("windows")) ? "7zip" : "tar"
    archiver.outputDirectory: product.destinationDirectory + "/../.."

    Depends {
        productTypes: [
            "application",
            "dynamiclibrary",
            "qm",
            "installable",
        ]
    }
}
