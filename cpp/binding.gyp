
{
  "targets": [
    {
      "target_name": "transcriberaddon",
      "sources": [  "TranscriberAddon.cpp", 
                    "BatchRetrieveWorker.cpp", 
                    "MasterQueue.h", "MasterQueue.cpp", 
                    "BatchWraper.h", 
                    "BatchWraperTranscription.h", "BatchWraperTranscription.cpp", 
                    "Knowledge.h","Knowledge.cpp", 
                    "Lexicon.h", "Lexicon.cpp", 
                    "MiscWorker.cpp", 
                    "CrowdCAT.h", "CrowdCAT.cpp", 
                    "Global.h", "Global.cpp", 
                    "Lexicon.h", "Lexicon.cpp", 
                    "batches.h", "batches.cpp", 
                    "spotting.h", 
                    "NetSpotter.h", "NetSpotter.cpp",
                    "Word.h", "Word.cpp",
                    "CorpusDataset.h"

                ],
      "_former_sources":[
                    "SpecialInstances.h",
                    "TestingInstances.h", "TestingInstances.cpp",
                    "TrainingInstances.h", "TrainingInstances.cpp",
                    "TrainingBatchWraperTranscription.h", "TrainingBatchWraperTranscription.cpp",
                    "SpecialBatchRetrieveWorker.cpp"
                ],
      "cflags": ["-Wall", "-std=c++11", "-fexceptions", "-DOPENCV2"],
      'cflags!': [ '-fno-exceptions', '-fno-rtti' ],
      'cflags_cc!': [ '-fno-exceptions', '-fno-rtti' ],
      "include_dirs" : [
            "<!(node -e \"require('nan')\")", 
            "/home/brian/Projects/brian_caffe/scripts/cnnspp_spotter",
            "/home/brian/Projects/brian_caffe/include"
          ],
      "libraries": [
            "-lopencv_highgui", "-lb64", "-pthread", "-lopencv_imgproc", "-fopenmp", 
            "-L/home/brian/Projects/brian_caffe/scripts/cnnspp_spotter", "-lcnnspp_spotter",
            "-L/home/brian/Projects/brian_caffe/build/lib", "-lcaffe", "-l boost_system"
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
