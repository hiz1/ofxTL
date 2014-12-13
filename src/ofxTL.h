//
//  ofxTL.h
//  katamichi001
//
//  Created by hiz on 2014/12/03.
//
//

#ifndef katamichi001_TL_h
#define katamichi001_TL_h

#include "ofMain.h"

class ofxTL;

// DONE:Sequenceを順番に実行
// DONE:入力処理
// DONE:Sequenceの返値。Seq終了時に値を返す
// DONE:Sequenceの途中で別Sequenceに移行←返値によってTL側で次のSeqを決める
// DONE:Sequence終了時の処理をSeqと戻り値によって変更
// DONE:TLの並列実行
// TODO:TL間のメッセージング
// TODO:Sequenceの親子関係（必要？）

class ofxSeq {
public:
    ofxSeq(ofxTL &tl, string seqId) : _tl(tl), _seqId(seqId){

    }
    virtual void setup(const ofParameterGroup *parm) {
        _frame = 0;
    }
    virtual bool update(ofParameterGroup *param) {
        _frame ++;
        return false;
    }
    int    frame() { return _frame;}
    ofxTL &tl() { return _tl;}
    const string &seqId() const {return _seqId;}
    virtual void keyPressed(int key) {}
    virtual void keyReleased(int key) {}
    virtual void mouseMoved(int x, int y ) {}
    virtual void mouseDragged(int x, int y, int button) {}
    virtual void mousePressed(int x, int y, int button) {}
    virtual void mouseReleased(int x, int y, int button) {}
    virtual void gotMessage(ofMessage msg) {}
private:
    int    _frame;
    ofxTL &_tl;
    string _seqId;
};

class ofxTL {
public:
    // 指定フレーム数だけウエイトするSeq
    class WaitSeq : public ofxSeq {
    public:
        typedef ofxSeq base;
        
        WaitSeq(ofxTL &phase, string seqId, int waitFrame)
        : ofxSeq(phase, seqId), waitFrame(waitFrame) {}
        
        virtual bool update(ofParameterGroup *param) {
            ofParameterGroup result;
            base::update(param);
            
            if(frame() >= waitFrame) {
                return true;
            }
            return false;
        }
    private:
        const int waitFrame;
    };
    
    // キー入力待ちするSeq
    class InputSeq : public ofxSeq {
    public:
        typedef ofxSeq base;
        
        InputSeq(ofxTL &phase, string seqId, vector<int> acceptKeys, int waitFrame = -1)
        : ofxSeq(phase, seqId),acceptKeys(acceptKeys), waitFrame(waitFrame) {}
        
        virtual void setup(const ofParameterGroup *parm) {
            base::setup(parm);
            inputedKey = -1;
        }
        
        virtual bool update(ofParameterGroup *parm) {
            base::update(parm);
            if(inputedKey >= 0) {
                ofParameter<int> keyParm("key", inputedKey);
                parm->add(keyParm);
                return true;
            }
            if(waitFrame >= 0 && frame() >= waitFrame)return new string();
            return false;
        }
        
        virtual void keyPressed(int key) {
            vector< int >::iterator cIter = find( acceptKeys.begin(),acceptKeys.end() , key );
            if(cIter != acceptKeys.end()) inputedKey = key;
        }
    private:
        int waitFrame;
        vector<int> acceptKeys;
        int  inputedKey;
    };
    
    ofxTL() {
        currentSeq  = NULL;
        frameCount = 0;

        
    }
    ~ofxTL() {
        for(vector<ofxSeq*>::iterator it=sequences.begin();it!=sequences.end();it++) {
            delete *it;
        }
    }
    virtual bool update() {
        frameCount ++;
        if(currentSeq != NULL) {
            ofParameterGroup parm;
            if(currentSeq->update(&parm)) {
                setSeqIdx(getNextSeqIdx(currentSeq, &parm), &parm);
            }
        }
        return false;
    }
    virtual void draw() {}
    virtual int getNextSeqIdx(ofxSeq *seq, ofParameterGroup *parm) {
        int nextSeq = seqIdx + 1;
        if(nextSeq >= sequences.size()) nextSeq = 0;
        return nextSeq;
    }
    virtual void setupSeq(ofParameterGroup *parm) {
        currentSeq->setup(parm);
    }
    int getFrameCount() {
        return frameCount;
    }
    void addSequence(ofxSeq *seq) {
        sequences.push_back(seq);
    }
    void setSeqIdx(int idx, ofParameterGroup *parm) {
        seqIdx = idx;
        currentSeq = sequences[idx];
        setupSeq(parm);
    }
    int getSeqIdxWithId(const string &id) {
        for(int i=0;i<sequences.size();i++) {
            if(sequences[i]->seqId() == id) return i;
        }
        return -1;
    }
    const ofxSeq &getSeq() {
        return *currentSeq;
    }
    virtual void keyPressed(int key) {
        currentSeq->keyPressed(key);
    }
    virtual void keyReleased(int key) {
        currentSeq->keyReleased(key);
    }
    virtual void mouseMoved(int x, int y ) {
        currentSeq->mouseMoved(x, y);
    }
    virtual void mouseDragged(int x, int y, int button) {
        currentSeq->mouseDragged(x,y,button);
    }
    virtual void mousePressed(int x, int y, int button) {
        currentSeq->mousePressed(x,y,button);
    }
    virtual void mouseReleased(int x, int y, int button) {
        currentSeq->mouseReleased(x,y,button);
    }
    virtual void gotMessage(ofMessage msg) {
        currentSeq->gotMessage(msg);
    }
private:
    int   frameCount;
    vector<ofxSeq*> sequences;
    int   seqIdx;
    ofxSeq  *currentSeq;
};


#endif

