using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class DisplayMyIDText : MonoBehaviour {

    int mMyID;
    NetworkMgr mNetwork;
    public Text text;

	// Use this for initialization
	void Start () {
        mMyID = (int)NetConfig.NIL;
        mNetwork = NetworkMgr.GetInstance();
        if (mNetwork != null)
            mMyID = mNetwork.GetMyID();
        this.ShowMyID();
	}
	
	// Update is called once per frame
	void Update () {
        if ((int)NetConfig.NIL == mMyID)
        {
            mMyID = mNetwork.GetMyID();
            this.ShowMyID();
        }
	}

    void ShowMyID() {
       this.text.text  = "My ID No." + mMyID.ToString();
    }
}
