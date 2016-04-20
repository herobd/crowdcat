#include "MasterQueue.h"

MasterQueue::MasterQueue() {
    sem_init(&mutexSem,false,1);
    atID=0;
    //For demo
    Spottings an1("an");
    an1.push_back(SpottingImage("test_an_0","an",cv::imread("test/bigramExamples/an/wordimg_7.png")));
    an1.push_back(SpottingImage("test_an_1","an",cv::imread("test/bigramExamples/an/wordimg_15.png")));
    an1.push_back(SpottingImage("test_an_2","an",cv::imread("test/bigramExamples/an/wordimg_66.png")));
    an1.push_back(SpottingImage("test_an_3","an",cv::imread("test/bigramExamples/an/wordimg_77.png")));
    an1.push_back(SpottingImage("test_an_4","an",cv::imread("test/bigramExamples/an/wordimg_145.png")));
    an1.push_back(SpottingImage("test_an_5","an",cv::imread("test/bigramExamples/an/wordimg_194.png")));
    an1.push_back(SpottingImage("test_an_6","an",cv::imread("test/bigramExamples/an/wordimg_208.png")));
    an1.push_back(SpottingImage("test_an_7","an",cv::imread("test/bigramExamples/an/wordimg_247.png")));
    an1.push_back(SpottingImage("test_an_8","an",cv::imread("test/bigramExamples/an/wordimg_263.png")));
    an1.push_back(SpottingImage("test_an_9","an",cv::imread("test/bigramExamples/an/wordimg_291.png")));
    spottingsQueue.push(an1);
    
    Spottings ed1("ed");
    ed1.push_back(SpottingImage("test_ed_0","an",cv::imread("test/bigramExamples/ed/wordimg_46.png")));
    ed1.push_back(SpottingImage("test_ed_1","an",cv::imread("test/bigramExamples/ed/wordimg_58.png")));
    ed1.push_back(SpottingImage("test_ed_2","an",cv::imread("test/bigramExamples/ed/wordimg_91.png")));
    ed1.push_back(SpottingImage("test_ed_3","an",cv::imread("test/bigramExamples/ed/wordimg_100.png")));
    ed1.push_back(SpottingImage("test_ed_4","an",cv::imread("test/bigramExamples/ed/wordimg_131.png")));
    ed1.push_back(SpottingImage("test_ed_5","an",cv::imread("test/bigramExamples/ed/wordimg_159.png")));
    ed1.push_back(SpottingImage("test_ed_6","an",cv::imread("test/bigramExamples/ed/wordimg_225.png")));
    ed1.push_back(SpottingImage("test_ed_7","an",cv::imread("test/bigramExamples/ed/wordimg_291.png")));
    ed1.push_back(SpottingImage("test_ed_8","an",cv::imread("test/bigramExamples/ed/wordimg_324.png")));
    ed1.push_back(SpottingImage("test_ed_9","an",cv::imread("test/bigramExamples/ed/wordimg_358.png")));
    spottingsQueue.push(ed1);
    
    Spottings th1("th");
    th1.push_back(SpottingImage("test_th_0","an",cv::imread("test/bigramExamples/th/wordimg_5.png")));
    th1.push_back(SpottingImage("test_th_1","an",cv::imread("test/bigramExamples/th/wordimg_13.png")));
    th1.push_back(SpottingImage("test_th_2","an",cv::imread("test/bigramExamples/th/wordimg_16.png")));
    th1.push_back(SpottingImage("test_th_3","an",cv::imread("test/bigramExamples/th/wordimg_18.png")));
    th1.push_back(SpottingImage("test_th_4","an",cv::imread("test/bigramExamples/th/wordimg_25.png")));
    th1.push_back(SpottingImage("test_th_5","an",cv::imread("test/bigramExamples/th/wordimg_28.png")));
    th1.push_back(SpottingImage("test_th_6","an",cv::imread("test/bigramExamples/th/wordimg_32.png")));
    th1.push_back(SpottingImage("test_th_7","an",cv::imread("test/bigramExamples/th/wordimg_35.png")));
    th1.push_back(SpottingImage("test_th_8","an",cv::imread("test/bigramExamples/th/wordimg_39.png")));
    th1.push_back(SpottingImage("test_th_9","an",cv::imread("test/bigramExamples/th/wordimg_42.png")));
    th1.push_back(SpottingImage("test_th_10","an",cv::imread("test/bigramExamples/th/wordimg_60.png")));
    th1.push_back(SpottingImage("test_th_11","an",cv::imread("test/bigramExamples/th/wordimg_87.png")));
    th1.push_back(SpottingImage("test_th_12","an",cv::imread("test/bigramExamples/th/wordimg_113.png")));
    th1.push_back(SpottingImage("test_th_13","an",cv::imread("test/bigramExamples/th/wordimg_125.png")));
    th1.push_back(SpottingImage("test_th_14","an",cv::imread("test/bigramExamples/th/wordimg_130.png")));
    th1.push_back(SpottingImage("test_th_15","an",cv::imread("test/bigramExamples/th/wordimg_132.png")));
    th1.push_back(SpottingImage("test_th_16","an",cv::imread("test/bigramExamples/th/wordimg_148.png")));
    th1.push_back(SpottingImage("test_th_17","an",cv::imread("test/bigramExamples/th/wordimg_151.png")));
    th1.push_back(SpottingImage("test_th_18","an",cv::imread("test/bigramExamples/th/wordimg_186.png")));
    th1.push_back(SpottingImage("test_th_19","an",cv::imread("test/bigramExamples/th/wordimg_196.png")));
    spottingsQueue.push(th1);
    
    Spottings he1("he");
    he1.push_back(SpottingImage("test_he_0","an",cv::imread("test/bigramExamples/he/wordimg_13.png")));
    he1.push_back(SpottingImage("test_he_1","an",cv::imread("test/bigramExamples/he/wordimg_16.png")));
    he1.push_back(SpottingImage("test_he_2","an",cv::imread("test/bigramExamples/he/wordimg_19.png")));
    he1.push_back(SpottingImage("test_he_3","an",cv::imread("test/bigramExamples/he/wordimg_25.png")));
    he1.push_back(SpottingImage("test_he_4","an",cv::imread("test/bigramExamples/he/wordimg_28.png")));
    he1.push_back(SpottingImage("test_he_5","an",cv::imread("test/bigramExamples/he/wordimg_35.png")));
    he1.push_back(SpottingImage("test_he_6","an",cv::imread("test/bigramExamples/he/wordimg_42.png")));
    spottingsQueue.push(he1);
    
    Spottings an2("an");
    an2.push_back(SpottingImage("test_an_10","an",cv::imread("test/bigramExamples/an/wordimg_334.png")));
    an2.push_back(SpottingImage("test_an_11","an",cv::imread("test/bigramExamples/an/wordimg_346.png")));
    an2.push_back(SpottingImage("test_an_12","an",cv::imread("test/bigramExamples/an/wordimg_348.png")));
    an2.push_back(SpottingImage("test_an_13","an",cv::imread("test/bigramExamples/an/wordimg_393.png")));
    an2.push_back(SpottingImage("test_an_14","an",cv::imread("test/bigramExamples/an/wordimg_422.png")));
    an2.push_back(SpottingImage("test_an_15","an",cv::imread("test/bigramExamples/an/wordimg_433.png")));
    an2.push_back(SpottingImage("test_an_16","an",cv::imread("test/bigramExamples/an/wordimg_435.png")));
    an2.push_back(SpottingImage("test_an_17","an",cv::imread("test/bigramExamples/an/wordimg_450.png")));
    an2.push_back(SpottingImage("test_an_18","an",cv::imread("test/bigramExamples/an/wordimg_465.png")));
    an2.push_back(SpottingImage("test_an_19","an",cv::imread("test/bigramExamples/an/wordimg_483.png")));
    spottingsQueue.push(an2);
    
    Spottings en2("en");
    en2.push_back(SpottingImage("test_en_0","en",cv::imread("test/bigramExamples/en/wordimg_2.png")));
    en2.push_back(SpottingImage("test_en_1","en",cv::imread("test/bigramExamples/en/wordimg_40.png")));
    en2.push_back(SpottingImage("test_en_2","en",cv::imread("test/bigramExamples/en/wordimg_47.png")));
    en2.push_back(SpottingImage("test_en_3","en",cv::imread("test/bigramExamples/en/wordimg_66.png")));
    en2.push_back(SpottingImage("test_en_4","en",cv::imread("test/bigramExamples/en/wordimg_77.png")));
    en2.push_back(SpottingImage("test_en_5","en",cv::imread("test/bigramExamples/en/wordimg_85.png")));
    en2.push_back(SpottingImage("test_en_6","en",cv::imread("test/bigramExamples/en/wordimg_106.png")));
    en2.push_back(SpottingImage("test_en_7","en",cv::imread("test/bigramExamples/en/wordimg_136.png")));
    en2.push_back(SpottingImage("test_en_8","en",cv::imread("test/bigramExamples/en/wordimg_150.png")));
    en2.push_back(SpottingImage("test_en_9","en",cv::imread("test/bigramExamples/en/wordimg_195.png")));
    en2.push_back(SpottingImage("test_en_10","en",cv::imread("test/bigramExamples/en/wordimg_231.png")));
    en2.push_back(SpottingImage("test_en_11","en",cv::imread("test/bigramExamples/en/wordimg_257.png")));
    en2.push_back(SpottingImage("test_en_12","en",cv::imread("test/bigramExamples/en/wordimg_348.png")));
    en2.push_back(SpottingImage("test_en_13","en",cv::imread("test/bigramExamples/en/wordimg_402.png")));
    en2.push_back(SpottingImage("test_en_14","en",cv::imread("test/bigramExamples/en/wordimg_408.png")));
    en2.push_back(SpottingImage("test_en_15","en",cv::imread("test/bigramExamples/en/wordimg_497.png")));
    spottingsQueue.push(en2);
    
    Spottings st1("st");
    st1.push_back(SpottingImage("test_st_0","an",cv::imread("test/bigramExamples/st/wordimg_118.png")));
    st1.push_back(SpottingImage("test_st_1","an",cv::imread("test/bigramExamples/st/wordimg_124.png")));
    st1.push_back(SpottingImage("test_st_2","an",cv::imread("test/bigramExamples/st/wordimg_283.png")));
    st1.push_back(SpottingImage("test_st_3","an",cv::imread("test/bigramExamples/st/wordimg_366.png")));
    st1.push_back(SpottingImage("test_st_4","an",cv::imread("test/bigramExamples/st/wordimg_762.png")));
    st1.push_back(SpottingImage("test_st_5","an",cv::imread("test/bigramExamples/st/wordimg_813.png")));
    st1.push_back(SpottingImage("test_st_6","an",cv::imread("test/bigramExamples/st/wordimg_1010.png")));
    st1.push_back(SpottingImage("test_st_7","an",cv::imread("test/bigramExamples/st/wordimg_1013.png")));
    st1.push_back(SpottingImage("test_st_8","an",cv::imread("test/bigramExamples/st/wordimg_1023.png")));
    spottingsQueue.push(st1);
    
    Spottings th2("th");
    th2.push_back(SpottingImage("test_th_20","an",cv::imread("test/bigramExamples/th/wordimg_220.png")));
    th2.push_back(SpottingImage("test_th_21","an",cv::imread("test/bigramExamples/th/wordimg_223.png")));
    th2.push_back(SpottingImage("test_th_22","an",cv::imread("test/bigramExamples/th/wordimg_228.png")));
    th2.push_back(SpottingImage("test_th_23","an",cv::imread("test/bigramExamples/th/wordimg_239.png")));
    th2.push_back(SpottingImage("test_th_24","an",cv::imread("test/bigramExamples/th/wordimg_242.png")));
    th2.push_back(SpottingImage("test_th_25","an",cv::imread("test/bigramExamples/th/wordimg_249.png")));
    th2.push_back(SpottingImage("test_th_26","an",cv::imread("test/bigramExamples/th/wordimg_270.png")));
    th2.push_back(SpottingImage("test_th_27","an",cv::imread("test/bigramExamples/th/wordimg_282.png")));
    th2.push_back(SpottingImage("test_th_28","an",cv::imread("test/bigramExamples/th/wordimg_285.png")));
    th2.push_back(SpottingImage("test_th_29","an",cv::imread("test/bigramExamples/th/wordimg_307.png")));
    th2.push_back(SpottingImage("test_th_30","an",cv::imread("test/bigramExamples/th/wordimg_322.png")));
    th2.push_back(SpottingImage("test_th_31","an",cv::imread("test/bigramExamples/th/wordimg_337.png")));
    th2.push_back(SpottingImage("test_th_32","an",cv::imread("test/bigramExamples/th/wordimg_344.png")));
    th2.push_back(SpottingImage("test_th_33","an",cv::imread("test/bigramExamples/th/wordimg_353.png")));
    th2.push_back(SpottingImage("test_th_34","an",cv::imread("test/bigramExamples/th/wordimg_367.png")));
    th2.push_back(SpottingImage("test_th_35","an",cv::imread("test/bigramExamples/th/wordimg_371.png")));
    th2.push_back(SpottingImage("test_th_36","an",cv::imread("test/bigramExamples/th/wordimg_349.png")));
    spottingsQueue.push(th2);
}

Spottings MasterQueue::getBatch(unsigned int numberOfInstances, unsigned int maxWidth) {
    sem_wait(&mutexSem);
    Spottings batch(spottingsQueue.front().ngram,"test_batch_"+to_string(atID++)+"_"+spottingsQueue.front().ngram);
    batch.instances = spottingsQueue.front().getSpottings(numberOfInstances,maxWidth);
    if (spottingsQueue.front().instances.size()==0) {
        Spottings tmp = spottingsQueue.front();
        
        spottingsQueue.pop();
        
        tmp._reset();
        spottingsQueue.push(tmp);
    }
    sem_post(&mutexSem);
    return batch;
}
