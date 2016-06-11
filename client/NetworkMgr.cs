using UnityEngine;
using UnityEngine.UI;
using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Threading;

public class NetworkMgr : MonoBehaviour
{

    public static NetworkMgr mInstance = null;

    private NetworkLib mNetwork;
    private string mIPBuf;
    private Thread mThread;
    private bool mConnecting;
    private bool mConnectSyncComplete;

    private Transform mPlayerTransform;
    private GameObject mPlayerLight;
    public GameObject mPlayerPrefab;
    public GameObject mOtherPlayerPrefab;

    private int mMyID;
    public struct OtherClientInfo
    {
        public int id;
        public GameObject characterInstance;
    };
    public List<OtherClientInfo> mOtherClients;

    private string mDebugText;
    
    private class PreviousSavedData
    {
        private PlayerMoveData playerMoveData;
        private PlayerLightData playerLightData;
        private MonsterMoveData monsterMoveData;

        public bool IsSame(PlayerMoveData curData)
        {
            bool retval = true;

            retval &= (playerMoveData.id == curData.id) ? true : false;
            retval &= (Math.Abs(playerMoveData.posX - curData.posX) < 3.0f) ? true : false;
            retval &= (Math.Abs(playerMoveData.posY - curData.posY) < 3.0f) ? true : false;
            retval &= (Math.Abs(playerMoveData.posZ - curData.posZ) < 3.0f) ? true : false;
            retval &= (Math.Abs(playerMoveData.dirX - curData.dirX) < 3.0f) ? true : false;
            retval &= (Math.Abs(playerMoveData.dirY - curData.dirY) < 3.0f) ? true : false;
            retval &= (Math.Abs(playerMoveData.dirZ - curData.dirZ) < 3.0f) ? true : false;
            retval &= (Math.Abs(playerMoveData.horizental - curData.horizental) < 3.0f) ? true : false;
            retval &= (Math.Abs(playerMoveData.vertical - curData.vertical) < 3.0f) ? true : false;
            retval &= (playerMoveData.sneak == curData.sneak) ? true : false;

            return retval;
        }

        public bool IsSame(PlayerLightData curData)
        {
            bool retval = true;

            retval &= (playerLightData.id == curData.id) ? true : false;
            retval &= (playerLightData.on == curData.on) ? true : false;
            retval &= (Math.Abs(playerLightData.rotX - curData.rotX) < 0.3f) ? true : false;
            retval &= (Math.Abs(playerLightData.rotY - curData.rotY) < 0.3f) ? true : false;
            retval &= (Math.Abs(playerLightData.rotZ - curData.rotZ) < 0.3f) ? true : false;
            retval &= (Math.Abs(playerLightData.rotW - curData.rotW) < 0.3f) ? true : false;

            return retval;
        }

        public bool IsSame(MonsterMoveData curData)
        {
            bool retval = true;

            retval &= (Math.Abs(monsterMoveData.posX - curData.posX) < 1.0f) ? true : false;
            retval &= (Math.Abs(monsterMoveData.posY - curData.posY) < 1.0f) ? true : false;
            retval &= (Math.Abs(monsterMoveData.posZ - curData.posZ) < 1.0f) ? true : false;

            return retval;
        }

        public void SetData(PlayerMoveData curData)
        {
            playerMoveData = curData;
            return;
        }

        public void SetData(PlayerLightData curData)
        {
            playerLightData = curData;
            return;
        }

        public void SetData(MonsterMoveData curData)
        {
            monsterMoveData = curData;
            return;
        }
    };
    private PreviousSavedData mPreviousSavedData;
    

    // 수신 패킷 처리함수 델리게이트.
    public delegate void RecvNotifier(PacketType id, byte[] data);
    // 수신 패킷 분배 해시 테이블.
    private Dictionary<int, RecvNotifier> mNotifier;
    // 이벤트 통지 델리게이트.
    public delegate void EventHandler(NetEventState state);
    // 이벤트 핸들러
    private EventHandler mHandler;

    private Vector3 mMonsterCurPos;
    private Vector3 mMonsterPatrolPos;
    private GameObject enemy;

    void Awake()
    {
        if (NetworkMgr.mInstance != null)
            Destroy(this.gameObject);

        GetInstance();
        mNetwork = new NetworkLib();
        mConnectSyncComplete = false;
        mConnecting = false;
//        mIPBuf = "127.0.0.1";
        mIPBuf = GameObject.FindGameObjectWithTag("InputIP").GetComponent<NetworkInpormation>().serverIP;

        mPlayerTransform = null;
        mPlayerLight = null;

        mMyID = (int)NetConfig.NIL;
        mOtherClients = new List<OtherClientInfo>();
        mDebugText = "";
        mPreviousSavedData = new PreviousSavedData();

        mNotifier = new Dictionary<int, RecvNotifier>();

        mMonsterCurPos = Vector3.zero;
        mMonsterPatrolPos = Vector3.zero;
        enemy = GameObject.FindGameObjectWithTag(Tags.enemy);
    }

    // Use this for initialization
    void Start()
    {
        GameObject.FindGameObjectWithTag("InputIP").SetActive(false);
        Application.runInBackground = true;
        if (!mNetwork.Connect(mIPBuf, (int)NetConfig.SERVER_PORT))
            mDebugText = "Start::Connect Fail !!";
        else
        {
            RegisterReceiveNotification(PacketType.SetID, OnReceiveSetIDPacket);
            RegisterReceiveNotification(PacketType.Connect, OnReceiveConnectPacket);
            RegisterReceiveNotification(PacketType.Disconnect, OnReceiveDisconnectPacket);
            mNetwork.RegisterEventHandler(OnEventHandling);
            LaunchThread();
        }
    }

    // Update is called once per frame
    void Update()
    {
        byte[] packet = new byte[(int)NetConfig.MAX_BUFFER_SIZE];

        if (mNetwork.Receive(ref packet, packet.Length) != -1)
            ReceivePacket(packet);
    }

    void OnApplicationQuit()
    {
        mConnecting = false;
        mThread.Join();
        mNetwork.Disconnect();
    }

    void OnGUI()
    {
        GUI.Label(new Rect(20, 20, 400, 25), "" + mDebugText);
    }

    private void LaunchThread()
    {
        try
        {
            mThread = new Thread(new ThreadStart(Dispatch));
            mThread.Start();
            mConnecting = true;
        }
        catch
        {
            Debug.Log("Cannot launch thread.");
        }
        return;
    }

    private void Dispatch()
    {
        while (mConnecting)
        {
            mNetwork.Dispatch();

            Thread.Sleep(5);
        }

        return;
    }

    public void RegisterReceiveNotification(PacketType id, RecvNotifier notifier)
    {
        int index = (int)id;

        if (mNotifier.ContainsKey(index)) mNotifier.Remove(index);

        mNotifier.Add(index, notifier);
        return;
    }

    public void UnregisterReceiveNotification(PacketType id)
    {
        int index = (int)id;

        if (mNotifier.ContainsKey(index)) mNotifier.Remove(index);
        return;
    }

    public void RegisterEventHandler(EventHandler handler)
    {
        mHandler += handler;
        return;
    }

    public void UnregisterEventHandler(EventHandler handler)
    {
        mHandler -= handler;
        return;
    }

    public void OnEventHandling(NetEventState state)
    {
        if (mHandler != null)
        {
            mHandler(state);
        }
        return;
    }

    private int SendReliable<T>(IPacket<T> packet)
    {
        int nSendSize = 0;

        if (mNetwork != null)
        {
            PacketHeader header = new PacketHeader();
            HeaderSerializer serializer = new HeaderSerializer();

            byte[] packetData = packet.GetByteData();

            header.size = (byte)(sizeof(byte) + sizeof(byte) + packetData.Length);
            header.type = packet.GetPacketType();

            byte[] headerData = null;
            if (serializer.Serialize(header) == false) return 0;
            headerData = serializer.GetSerializedData();

            byte[] data = new byte[headerData.Length + packetData.Length];
            int headerSize = Marshal.SizeOf(typeof(PacketHeader));
            Buffer.BlockCopy(headerData, 0, data, 0, headerSize);
            Buffer.BlockCopy(packetData, 0, data, headerSize, packetData.Length);

            nSendSize = mNetwork.Send(data, data.Length);
        }

        return nSendSize;
    }

    private void ReceivePacket(byte[] data)
    {
        PacketHeader header = new PacketHeader();
        HeaderSerializer serializer = new HeaderSerializer();

        bool ret = serializer.Deserialize(data, ref header);
        if (false == ret) return;
        // Packet으로서 인식할 수 없으므로 폐기합니다.

        byte packetType = header.type;

        if (mNotifier.ContainsKey(packetType) && mNotifier[packetType] != null)
        {
            int headerSize = Marshal.SizeOf(typeof(PacketHeader)); //sizeof(byte) + sizeof(byte);
            byte[] packetData = new byte[data.Length - headerSize];
            Buffer.BlockCopy(data, headerSize, packetData, 0, packetData.Length);

            mNotifier[(int)packetType]((PacketType)packetType, packetData);
        }
        return;
    }

    public void CreatePlayer(int cloneID, Vector3 pos)
    {
        // 네트워크 상에 플레이어를 동적 생성 함수       

        if (cloneID == mMyID)
        {
            mPlayerTransform = GameObject.FindGameObjectWithTag(Tags.player).GetComponent<Transform>();
            mPlayerLight = GameObject.FindGameObjectWithTag("FlashLight");
        }
        else
        {
            GameObject instance = (GameObject)Instantiate(mOtherPlayerPrefab, pos, Quaternion.identity);

            instance.transform.Find("Player_Camera").gameObject.SetActive(false);
            instance.transform.Find("OVRCameraController").gameObject.SetActive(false);
            instance.transform.gameObject.GetComponent<AudioListener>().enabled = false;
            instance.transform.gameObject.GetComponent<NavMeshAgent>().enabled = true;
            instance.transform.position = pos - instance.transform.gameObject.transform.position;
            instance.transform.gameObject.transform.position = pos;

            OtherClientInfo otherClientInfo;
            otherClientInfo.id = cloneID;
            otherClientInfo.characterInstance = instance;

            mOtherClients.Add(otherClientInfo);

            instance.transform.gameObject.GetComponent<PlayerMovement>().mInstanceID = cloneID;
        }

        return;
    }

    private void OnReceiveSetIDPacket(PacketType type, byte[] packetData)
    {
        SetIDPacket packet = new SetIDPacket(packetData);
        SetIDData data = packet.GetPacketData();
        mMyID = data.id;

        CreatePlayer(data.id, new Vector3(263.0f, -14.0f, -2.0f));

        // 함수 동작 후 결과를 통해 플레이어 객체에 ID 부여
        if (true == SendConnectPacket())
            GameObject.FindGameObjectWithTag(Tags.player).GetComponent<PlayerMovement>().mInstanceID = mMyID;

        return;
    }

    private bool SendConnectPacket()
    {
        try
        {
            ConnectData data = new ConnectData();
            data.id = mMyID;
            data.posX = mPlayerTransform.position.x;
            data.posY = mPlayerTransform.position.y;
            data.posZ = mPlayerTransform.position.z;

            ConnectPacket packet = new ConnectPacket(data);
            SendReliable(packet);
        }
        catch
        {
            if (mHandler != null)
            {
                NetEventState state = new NetEventState();
                state.mType = NetEventType.SendError;
                state.mResult = NetEventResult.Failure;
                mHandler(state);
                return false;
            }
        }
        return true;
    }

    private void OnReceiveConnectPacket(PacketType type, byte[] packetData)
    {
        ConnectPacket packet = new ConnectPacket(packetData);
        ConnectData data = packet.GetPacketData();

        PlayerInfo info = new PlayerInfo();
        info.id = data.id;

        if (info.id != mMyID)
        {
            info.pos.x = data.posX;
            info.pos.y = data.posY;
            info.pos.z = data.posZ;

            CreatePlayer(info.id, info.pos);
        }

        return;
    }

    private void OnReceiveDisconnectPacket(PacketType type, byte[] packetData)
    {
        DisconnectPacket packet = new DisconnectPacket(packetData);
        DisconnectData data = packet.GetPacketData();

        if (data.id != mMyID)
        {
            int index = mOtherClients.FindIndex(
                    delegate (OtherClientInfo i)
                    {
                        return i.id == data.id;
                    });
            if (-1 != index)
            {
                DestroyObject(mOtherClients[index].characterInstance);
                mOtherClients.RemoveAt(index);
            }
        }

        return;
    }

    public void SendPlayerMovePacket(Vector3 dir, float h, float v, bool s, Vector3 playerPosition)
    {
        if (!mConnectSyncComplete) return;

        PlayerMoveData data = new PlayerMoveData();
        data.id = mMyID;
        data.posX = playerPosition.x;
        data.posY = playerPosition.y;
        data.posZ = playerPosition.z;
        data.dirX = dir.x;
        data.dirY = dir.y;
        data.dirZ = dir.z;
        data.horizental = h;
        data.vertical = v;
        data.sneak = s;

        if (!mPreviousSavedData.IsSame(data))
        {
            PlayerMovePacket packet = new PlayerMovePacket(data);
            SendReliable(packet);
            mPreviousSavedData.SetData(data);
        }
        return;
    }

    public void SendMonsterMovePacket(Vector3 pos)
    {
        if (!mConnectSyncComplete) return;

        MonsterMoveData data = new MonsterMoveData();
        data.posX = pos.x;
        data.posY = pos.y;
        data.posZ = pos.z;

        if (!mPreviousSavedData.IsSame(data))
        {
            MonsterMovePacket packet = new MonsterMovePacket(data);
            SendReliable(packet);
            mPreviousSavedData.SetData(data);
        }

        return;
    }

    public void SendPlayerLightPacket(bool on, Quaternion rotation)
    {
        if (!mConnectSyncComplete) return;

        PlayerLightData data = new PlayerLightData();
        data.id = mMyID;
        data.on = on;
        data.rotX = rotation.x;
        data.rotY = rotation.y;
        data.rotZ = rotation.z;
        data.rotW = rotation.w;

        if (!mPreviousSavedData.IsSame(data))
        {
            PlayerLightPacket packet = new PlayerLightPacket(data);
            SendReliable(packet);
            mPreviousSavedData.SetData(data);
        }

        return;
    }

    public void SendPlayerShoutPacket(bool shouting, Vector3 position)
    {
        if (!mConnectSyncComplete) return;

        PlayerShoutData data = new PlayerShoutData();
        data.id = mMyID;
        data.shouting = shouting;
        data.posX = position.x;
        data.posY = position.y;
        data.posZ = position.z;

        PlayerShoutPacket packet = new PlayerShoutPacket(data);
        SendReliable(packet);

        return;
    }

    public static NetworkMgr GetInstance()
    {
        if (NetworkMgr.mInstance == null)
        {
            NetworkMgr.mInstance = GameObject.Find("NetworkManager").GetComponent<NetworkMgr>();
        }

        return (NetworkMgr.mInstance);
    }

    public int GetMyID()
    {
        return mMyID;
    }

    public void CompeleteConnect()
    {
        mConnectSyncComplete = true;
        Debug.Log("Connect synchronization complete!!!");
        return;
    }
}