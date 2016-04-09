
{
  "targets": [
    {
      "target_name": "spottingaddon",
      "sources": [ "SpottingAddon.cpp", "BatchRetrieveWorker.cpp", "SpottingBatchUpdateWorker.cpp", "MasterQueue.h", "MasterQueue.cpp"],
      "cflags": ["-Wall", "-std=c++11"],
      "include_dirs" : ["<!(node -e \"require('nan')\")"],
      "libraries": [
            "-lopencv_highgui", "-lb64", "-lpthread"
          ],
      "conditions": [
        [ 'OS=="mac"', {
            "xcode_settings": {
                'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11','-stdlib=libc++'],
                'OTHER_LDFLAGS': ['-stdlib=libc++'],
                'MACOSX_DEPLOYMENT_TARGET': '10.7' }
            }
        ]
      ]
    }
  ]
}
