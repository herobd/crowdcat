
{
  "targets": [
    {
      "target_name": "spottingaddon",
      "sources": [ "SpottingAddon.cpp", "BatchRetrieveWorker.cpp", "SpottingBatchUpdateWorker.cpp", "MasterQueue.h", "MasterQueue.cpp", "SpottingResults.h", "SpottingResults.cpp", "TestQueue.h", "TestQueue.cpp", "BatchWraper.h", "BatchWraperSpottings.h", "BatchWraperSpottings.cpp", "BatchWraperTranscription.h", "BatchWraperTranscription.cpp", "TranscribeBatchQueue.h", "TranscribeBatchQueue.cpp", "Knowledge.h","Knowledge.cpp", "Lexicon.h", "Lexicon.cpp", "MiscWorker.cpp", "CATTSS.h", "CATTSS.cpp", "Spotter.h", "FacadeSpotter.h", "Spotter.cpp", "FacadeSpotter.cpp", "Global.h", "Global.cpp", "Lexicon.h", "Lexicon.cpp", "NewExemplarsBatchQueue.h", "NewExemplarsBatchQueue.cpp", "BatchWraperNewExemplars.h", "BatchWraperNewExemplars.cpp", "NewExemplarsBatchUpdateWorker.cpp", "maxflow/graph.h", "maxflow/graph.cpp", "maxflow/block.h", "maxflow/instances.inc", "maxflow/maxflow.cpp" ],
      "cflags": ["-Wall", "-std=c++11", "-fexceptions" ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      "include_dirs" : ["<!(node -e \"require('nan')\")"],
      "libraries": [
            "-lopencv_highgui", "-lb64", "-lpthread", "-lopencv_imgproc", "-fopenmp"
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
