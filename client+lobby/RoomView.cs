using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class RoomView : MonoBehaviour {

    public Canvas panelGameRoomCanvas;
    private NetworkMgr mNetwork;

    public GameObject scrollContents;
    public GameObject roomItem;
    public GameObject userItem;
    public GameObject gameRoomPanel; 

    int mCurrentRoomNo;
    int mCurrentChiefNo;

    // Grid Layout Group 컴포넌트의 Constraint Count 값을 증가시킬 변수
    int mItemRowCount;

    void Awake()
    {
        mItemRowCount = 0;
        mCurrentRoomNo = 0;
        mCurrentChiefNo = (int)NetConfig.NIL;
        scrollContents.GetComponent<RectTransform>().sizeDelta = Vector2.zero;
    }

	// Use this for initialization
	void Start () {
        mNetwork = NetworkMgr.GetInstance();

        mNetwork.RegisterReceiveNotification(PacketType.CreateRoom, OnReceiveCreateRoomPacket);
        mNetwork.RegisterReceiveNotification(PacketType.RenewalRoomInfo, OnRecvRenewalRoomInfoPacket);
        mNetwork.RegisterNoticeNotification(Notice.JOIN_FAIL, OnNotifyJoinFail);
        mNetwork.RegisterNoticeNotification(Notice.UPDATE_ROOM, OnNotifyUpdateRoom);
    }

    // Update is called once per frame
    void Update () {
	    
	}

    private void OnNotifyGameStart()
    {
        Debug.Log("On Notify Game Start ^^*");

        // 게임 씬으로 넘어가면 된다. 게임에 필요한 데이터를 처리받는 부분이 될 수도 있다.

        return;
    }

    private void OnNotifyJoinFail()
    {
        Debug.Log("On Notify Join Fail :(");

        return;
    }

    private void OnNotifyUpdateRoom()
    {
        Debug.Log("On Notify Game Start ㅇ3ㅇ");

        CleanGameRoomItem();

        return;
    }

    private void OnRecvRenewalRoomInfoPacket(PacketType type, byte[] packetData)
    {
        CleanUserItem();
        RenewalRoomInfoPacket packet = new RenewalRoomInfoPacket(packetData);
        RenewalRoomInfoData data = packet.GetPacketData();

        // 방장 정보 갱신
        mCurrentChiefNo = data.chiefNo;

        // userItem 프리팹을 동적으로 생성.
        GameObject chief = (GameObject)Instantiate(userItem);
        // 생성한 userItem 프리팹의 Parent를 지정.
        chief.transform.SetParent(gameRoomPanel.transform, false);

        UserData chiefData = chief.GetComponent<UserData>();
        chiefData.userID = data.chiefNo;
        chiefData.isReady = false;
        chiefData.DisplayUserData();
        
        if (data.partner_1_ID != (int)NetConfig.NIL)
        {
            GameObject partner_1 = (GameObject)Instantiate(userItem);
            partner_1.transform.SetParent(gameRoomPanel.transform, false);

            UserData partner_1_Data = partner_1.GetComponent<UserData>();
            partner_1_Data.userID = data.partner_1_ID;
            partner_1_Data.isReady = data.partner_1_ready;
            partner_1_Data.DisplayUserData();
        }
        if (data.partner_2_ID != (int)NetConfig.NIL)
        {
            GameObject partner_2 = (GameObject)Instantiate(userItem);
            partner_2.transform.SetParent(gameRoomPanel.transform, false);

            UserData partner_2_Data = partner_2.GetComponent<UserData>();
            partner_2_Data.userID = data.partner_2_ID;
            partner_2_Data.isReady = data.partner_2_ready;
            partner_2_Data.DisplayUserData();
        }
        if (data.partner_3_ID != (int)NetConfig.NIL)
        {
            GameObject partner_3 = (GameObject)Instantiate(userItem);
            partner_3.transform.SetParent(gameRoomPanel.transform, false);

            UserData partner_3_Data = partner_3.GetComponent<UserData>();
            partner_3_Data.userID = data.partner_3_ID;
            partner_3_Data.isReady = data.partner_3_ready;
            partner_3_Data.DisplayUserData();
        }
        
        Debug.Log("On Recv Renewal RoomInfo Packet!!");

        return;
    }

    private void OnReceiveCreateRoomPacket(PacketType type, byte[] packetData)
    {
        CreateRoomPacket packet = new CreateRoomPacket(packetData);
        CreateRoomData data = packet.GetPacketData();

        // RoomItem 프리팹을 동적으로 생성
        GameObject room = (GameObject)Instantiate(roomItem);
        // 생성한 RoomItem 프리팹의 Parent를 지정
        room.transform.SetParent(scrollContents.transform, false);

        RoomData roomData = room.GetComponent<RoomData>();
        roomData.roomNo = data.roomNo;
        roomData.chiefNo = data.chiefNo;
        roomData.maxPlayers = 4;
        roomData.connectPlayer = 1;

        //Debug.Log(data.partner_1_ID + ", " + data.partner_2_ID + ", " + data.partner_3_ID);

        if (data.partner_1_ID != (int)NetConfig.NIL)
            roomData.connectPlayer++;
        if (data.partner_2_ID != (int)NetConfig.NIL)
            roomData.connectPlayer++;
        if (data.partner_3_ID != (int)NetConfig.NIL)
            roomData.connectPlayer++;
        
        roomData.DisplayRoomData();

        // RoomItem의 Button 컴포넌트에 클릭 이벤트를 동적으로 연결
        roomData.GetComponent<UnityEngine.UI.Button>().onClick.AddListener(delegate { OnClickRoomItem(data.roomNo); });

        Debug.Log("Recv CreateRoom Packet : Room No." + data.roomNo + " /Chief No." + data.chiefNo);

        // Grid Layout Group 컴포넌트의 Constraint Count 값을 증가시킴
        scrollContents.GetComponent<GridLayoutGroup>().constraintCount = ++mItemRowCount;
        // 스크롤 영역의 높이를 증가시킴
        scrollContents.GetComponent<RectTransform>().sizeDelta += new Vector2(0, 35);

        return;
    }

    public void OnClickRoomItem(int roomNo)
    {
        if (mCurrentRoomNo != roomNo)
            mCurrentRoomNo = roomNo;
        else
            mCurrentRoomNo = 0;

        Debug.Log("On Click RoomItem : " + mCurrentRoomNo);

        return;
    }

    public void OnClickJoinRoomButton()
    {
        if (mCurrentRoomNo == 0) return;

        RoomData roomData = null;
        foreach (GameObject obj in GameObject.FindGameObjectsWithTag("ROOM_ITEM"))
        {
            if (obj.GetComponent<RoomData>().roomNo == mCurrentRoomNo)
            {
                roomData = obj.GetComponent<RoomData>();
                break;
            }
        }

        Debug.Log("OnClick Join Room Button : " + mCurrentChiefNo + ", " + roomData.chiefNo);
        mCurrentChiefNo = roomData.chiefNo;

        if (roomData.connectPlayer >= 4)
        {
            mCurrentRoomNo = 0;
            mCurrentChiefNo = (int)NetConfig.NIL;
            return;
        }

        panelGameRoomCanvas.enabled = true;
        mNetwork.SendJoinRoomPacket(mCurrentRoomNo);
        return;
    }

    public void OnClickMakeGameRoom()
    {
        CleanGameRoomItem();
        mCurrentChiefNo = mNetwork.GetMyID();
        panelGameRoomCanvas.enabled = true;
        
        // userItem 프리팹을 동적으로 생성.
        GameObject user = (GameObject)Instantiate(userItem);
        // 생성한 userItem 프리팹의 Parent를 지정.
        user.transform.SetParent(gameRoomPanel.transform, false);

        UserData userData = user.GetComponent<UserData>();
        userData.userID = mNetwork.GetMyID();
        userData.isReady = false;
        userData.DisplayUserData();

        mNetwork.SendNotifyPacket((int)Notice.MAKE_ROOM);
        return;
    }

    public void CloseGameRoom()
    {
        panelGameRoomCanvas.enabled = false;
        CleanUserItem();
        mCurrentRoomNo = 0;
        mCurrentChiefNo = (int)NetConfig.NIL;
        mNetwork.SendNotifyPacket((int)Notice.QUIT_ROOM);
        return;
    }

    public void OnClickGameStartButton()
    {
        bool all_ready = true;
        
        if (mCurrentChiefNo == mNetwork.GetMyID()) {
            foreach (GameObject obj in GameObject.FindGameObjectsWithTag("USER_ITEM"))
            {
                UserData userData = obj.GetComponent<UserData>();
                if (userData.userID != mNetwork.GetMyID())
                    if (!userData.isReady) all_ready = false;
            }

            if (all_ready) mNetwork.SendNotifyPacket((int)Notice.GAME_READY);
        }
        else {
            foreach (GameObject obj in GameObject.FindGameObjectsWithTag("USER_ITEM"))
            {
                UserData userData = obj.GetComponent<UserData>();
                if (userData.userID == mNetwork.GetMyID())
                {
                    userData.isReady = !userData.isReady;
                    userData.DisplayUserData();

                    if (userData.isReady)
                        mNetwork.SendNotifyPacket((int)Notice.GAME_READY);
                    else
                        mNetwork.SendNotifyPacket((int)Notice.CANCEL_READY);

                    break;
                }
            }
        }
        return;
    }

    public void OnClickQuitButton()
    {
        // title scene 으로 넘기는 동작을 수행
        return;
    }

    private void CleanGameRoomItem()
    {
        foreach (GameObject obj in GameObject.FindGameObjectsWithTag("ROOM_ITEM"))
            Destroy(obj);
    }

    private void CleanUserItem()
    {
        foreach (GameObject obj in GameObject.FindGameObjectsWithTag("USER_ITEM"))
            Destroy(obj);
    }
}
