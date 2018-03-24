import qbs

InstallPackage {
    targetName: "ThunderEngine-" + qbs.targetOS[0] + "-" + qbs.architecture

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
