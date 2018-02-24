var Utilities = require("qbs.Utilities");

function stripKernelReleaseSuffix(r) {
    var idx = r.indexOf("-RELEASE");
    return idx >= 0 ? r.substr(0, idx) : r;
}

function hostKernelRelease() {
    return stripKernelReleaseSuffix(Utilities.kernelVersion());
}
