self.addEventListener('message', function(e) {
    const file = e.data.blob;
    SDFGen(file, 1, 1);
}, false);

var Module = {
    'print': function(text) {
        self.postMessage({"log":text});
    }
};

self.importScripts("SDFgen.js");

let last_file_name = undefined;

function SDFGen(file, padding, dx) {

    var filename = file.name;

    // if simplify on the same file, don't even read the file
    if (filename === last_file_name) {
        console.log("skipping load and create data file");
        simplify(filename, percentage, filename);
        return;
    } else { // remove last file in memory
        if (last_file_name !== undefined)
            Module.FS_unlink(last_file_name);
    }

    last_file_name = filename;
    var fr = new FileReader();
    fr.readAsArrayBuffer(file);
    fr. onloadend = function (e) {
        var data = new Uint8Array(fr.result);
        Module.FS_createDataFile(".", filename, data, true, true);
        Module.ccall("SDFGen", // c function name
            undefined, // return
            ["string", "number", "number"], // param
            [filename, padding, dx]
        );

        // same hacky way of getting the name like in c++
        let out_bin = Module.FS_readFile(filename.slice(0, filename.length-3)+"sdf");
        // sla should work for binary stl
        let file = new Blob([out_bin], {type: 'application/sla'});
        self.postMessage({"blob":file});
    }
}
