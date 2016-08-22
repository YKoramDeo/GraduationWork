using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class UserData : MonoBehaviour {

    // 외부 접근을 위해 public 으로 선언했지만 Inspector에 노출하지 않음.
    [HideInInspector]
    public int userID;
    [HideInInspector]
    public bool isReady;

    public Text textUserID;
    public Text textReady;

    void Awake()
    {
        userID = (int)NetConfig.NIL;
        isReady = false;
    }

	public void DisplayUserData()
    {
        textUserID.text = "User." + userID.ToString();
        textReady.text = "Ready";
        textReady.enabled = (isReady) ? true : false;
        return;
    }
}
