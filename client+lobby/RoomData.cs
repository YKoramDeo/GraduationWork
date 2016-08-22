using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class RoomData : MonoBehaviour {

    // 외부 접근을 위해 public으로 선언했지만 Inspector에 노출하지 않음
    [HideInInspector]
    public int roomNo = 0;
    [HideInInspector]
    public int chiefNo = 0;
    [HideInInspector]
    public int connectPlayer = 0;
    [HideInInspector]
    public int maxPlayers = 0;

    // 룸 이름 표시할 Text UI 항목
    public Text textRoomName;
    // 룸 접속자 수와 최대 접속자 수를 표시할 Text UI 항목
    public Text textConnectInfo;

    // 룸 정보를 전달한 후 Text UI 항목에 표시하는 함수
    public void DisplayRoomData()
    {
        textRoomName.text = "Class #" + roomNo.ToString();
        textConnectInfo.text = "[" + connectPlayer.ToString() + "/" + maxPlayers.ToString() + "]";
        return;
    }
}
