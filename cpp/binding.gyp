
{
  "targets": [
    {
      "target_name": "spottingaddon",
      "sources": [  "SpottingAddon.cpp", 
                    "BatchRetrieveWorker.cpp", 
                    "SpottingBatchUpdateWorker.cpp", 
                    "MasterQueue.h", "MasterQueue.cpp", 
                    "SpottingResults.h", "SpottingResults.cpp", 
                    "BatchWraper.h", 
                    "BatchWraperSpottings.h", "BatchWraperSpottings.cpp", 
                    "BatchWraperTranscription.h", "BatchWraperTranscription.cpp", 
                    "TranscribeBatchQueue.h", "TranscribeBatchQueue.cpp", 
                    "Knowledge.h","Knowledge.cpp", 
                    "Lexicon.h", "Lexicon.cpp", 
                    "MiscWorker.cpp", 
                    "CATTSS.h", "CATTSS.cpp", 
                    "Global.h", "Global.cpp", 
                    "Lexicon.h", "Lexicon.cpp", 
                    "NewExemplarsBatchQueue.h", "NewExemplarsBatchQueue.cpp", 
                    "BatchWraperNewExemplars.h", "BatchWraperNewExemplars.cpp", 
                    "NewExemplarsBatchUpdateWorker.cpp", 
                    "maxflow/graph.h", "maxflow/graph.cpp", "maxflow/block.h", "maxflow/instances.inc", "maxflow/maxflow.cpp", 
                    "batches.h", "batches.cpp", 
                    "spotting.h", 
                    "CorpusRef.h", "PageRef.h",
                    "WordBackPointer.h",
                    "SpottingQuery.h"
                    "SpottingQueue.h", "SpottingQueue.cpp",
                    "Spotter.h", "AlmazanSpotter.h", "AlmazanSpotter.cpp",
                    "AlmazanDataset.h", "AlmazanDataset.cpp",

                    "SpecialInstances.h",
                    "TestingInstances.h", "TestingInstances.cpp",
                    "TrainingInstances.h", "TrainingInstances.cpp",
                    "TrainingBatchWraperSpottings.h", "TrainingBatchWraperSpottings.cpp",
                    "TrainingBatchWraperTranscription.h", "TrainingBatchWraperTranscription.cpp",
                    "SpecialBatchRetrieveWorker.cpp"
                ],
      "cflags": ["-Wall", "-std=c++11", "-fexceptions" ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      "include_dirs" : ["<!(node -e \"require('nan')\")", "/home/brian/intel_index/EmbAttSpotter"],
      "libraries": [
            "-lopencv_highgui", "-lb64", "-pthread", "-lopencv_imgproc", "-fopenmp", 
            "-L/home/brian/intel_index/EmbAttSpotter", "-lembattspotter",
            "-L/home/brian/intel_index/EmbAttSpotter/vlfeat-0.9.20/bin/glnxa64/", "-l:libvl.so"
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
