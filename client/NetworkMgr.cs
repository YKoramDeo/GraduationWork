using UnityEngine;
using UnityEngine.UI;
using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

public class NetworkMgr : MonoBehaviour {

    public static NetworkMgr mInstance = null;

    private NetworkLib mNetwork;
    private string mIPBuf;

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

        public bool Check(PlayerMoveData curData)
        {
            bool retval = true;

            retval &= (playerMoveData.id == curData.id) ? true : false;
            retval &= (playerMoveData.posX == curData.posX) ? true : false;
            retval &= (playerMoveData.posY == curData.posY) ? true : false;
            retval &= (playerMoveData.posZ == curData.posZ) ? true : false;
            retval &= (playerMoveData.dirX == curData.dirX) ? true : false;
            retval &= (playerMoveData.dirY == curData.dirY) ? true : false;
            retval &= (playerMoveData.dirZ == curData.dirZ) ? true : false;
            retval &= (playerMoveData.horizental == curData.horizental) ? true : false;
            retval &= (playerMoveData.vertical == curData.vertical) ? true : false;
            retval &= (playerMoveData.sneak == curData.sneak) ? true : false;

            return retval;
        }

        public bool Check(PlayerLightData curData)
        {
            bool retval = true;

            retval &= (playerLightData.id == curData.id) ? true : false;
            retval &= (playerLightData.on == curData.on) ? true : false;
            retval &= (playerLightData.rotX == curData.rotX) ? true : false;
            retval &= (playerLightData.rotY == curData.rotY) ? true : false;
            retval &= (playerLightData.rotZ == curData.rotZ) ? true : false;
            retval &= (playerLightData.rotW == curData.rotW) ? true : false;

            return retval;
        }

        public bool Check(MonsterMoveData curData)
        {
            bool retval = true;

            retval &= (monsterMoveData.posX == curData.posX) ? true : false;
            retval &= (monsterMoveData.posY == curData.posY) ? true : false;
            retval &= (monsterMoveData.posZ == curData.posZ) ? true : false;
            
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

    private Vector3 mMonsterCurPos;
    private Vector3 mMonsterPatrolPos;
    private GameObject enemy;

    void Awake()
    {
        if (NetworkMgr.mInstance != null)
            Destroy(this.gameObject);

        GetInstance();
        mNetwork = new NetworkLib();
        mIPBuf = "127.0.0.1";
        //mIPBuf = GameObject.FindGameObjectWithTag("InputIP").GetComponent<NetworkInpormation>().serverIP;

        mPlayerTransform = null;
        mPlayerLight = null;

        mMyID = (int)NetConfig.NIL; 
        mOtherClients = new List<OtherClientInfo>();
        mDebugText = "";
        mPreviousSavedData = new PreviousSavedData();

        mMonsterCurPos = Vector3.zero;
        mMonsterPatrolPos = Vector3.zero;
        enemy = GameObject.FindGameObjectWithTag(Tags.enemy);
    }

    // Use this for initialization
    void Start()
    {
        //GameObject.FindGameObjectWithTag("InputIP").SetActive(false);
        Application.runInBackground = true;
        if (!mNetwork.Connect(mIPBuf, (int)NetConfig.SERVER_PORT))
            mDebugText = "Start::Connect Fail !!";
    }

    // Update is called once per frame
    void Update()
    {
        if (mNetwork.HasData())
        {
            byte[] packet = new byte[(int)NetConfig.MAX_BUFFER_SIZE];

            mNetwork.Receive(ref packet, packet.Length);
            ReceivePacket(packet);
        }

    }

    void OnApplicationQuit()
    {
        //SendDisconnectPacket();
        mNetwork.Disconnect();
    }

    void OnGUI()
    {
        GUI.Label(new Rect(20, 20, 400, 25), "" + mDebugText);
    }

    public void CreatePlayer(int cloneID, Vector3 pos)
    { 
        // 네트워크 상에 플레이어를 동적 생성 함수       
       
        if (cloneID == mMyID)
        {
            mPlayerTransform = GameObject.FindGameObjectWithTag(Tags.player).GetComponent<Transform>();
            mPlayerLight = GameObject.FindGameObjectWithTag("FlashLight");

            GameObject.FindGameObjectWithTag(Tags.player).GetComponent<PlayerMovement>().mInstanceID = cloneID;
        }
        else
        {
            GameObject instance = (GameObject)Instantiate(mOtherPlayerPrefab, pos, Quaternion.identity);

            /*
            instance.transform.Find("Controll_Camera").gameObject.SetActive(false);
            instance.transform.Find("char_ethan").Find("Player_Camera").gameObject.SetActive(false);
            instance.transform.Find("char_ethan").Find("OVRCameraController").gameObject.SetActive(false);
            instance.transform.Find("char_ethan").gameObject.GetComponent<AudioListener>().enabled = false;
            instance.transform.Find("char_ethan").gameObject.GetComponent<NavMeshAgent>().enabled = true;
            instance.transform.position = pos - instance.transform.Find("char_ethan").gameObject.transform.position;
            instance.transform.Find("char_ethan").gameObject.transform.position = pos;
            */
            // instance.transform.Find("Controll_Camera").gameObject.SetActive(false);
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

            // instance.transform.Find("char_ethan").gameObject.GetComponent<PlayerMovement>().mInstanceID = cloneID;
            instance.transform.gameObject.GetComponent<PlayerMovement>().mInstanceID = cloneID;
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

        serializer.Deserialize(data, ref header);

        int headerSize = sizeof(byte) + sizeof(byte);
        byte packetType = header.type;

        byte[] packetData = new byte[data.Length - headerSize];

        Buffer.BlockCopy(data, headerSize, packetData, 0, packetData.Length);

        switch (packetType)
        {
            case (byte)PacketType.SetID:
                if(mMyID == (int)NetConfig.NIL)
                    OnReceiveSetIDPacket(packetData);
                break;
            case (byte)PacketType.Connect:
                OnReceiveConnectPacket(packetData);
                break;
            case (byte)PacketType.Disconnect:
                OnReceiveDisconnectPacket(packetData);
                break;
            case (byte)PacketType.PlayerMove:
                OnReceivePlayerMovePacket(packetData);
                break;
            case (byte)PacketType.PlayerLight:
                OnReceivePlayerLightPacket(packetData);
                break;
            case (byte)PacketType.PlayerShout:
                OnReceivePlayerShoutPacket(packetData);
                break;
            case (byte)PacketType.MonsterSetInfo:
                OnReceiveMonsterSetInfo(packetData);
                break;
        }

        return;
    }

    private void OnReceiveSetIDPacket(byte[] packetData)
    {
        SetIDPacket packet = new SetIDPacket(packetData);
        SetIDData data = packet.GetPacketData();
        mMyID = data.id;

        CreatePlayer(data.id, new Vector3(263.0f, -14.0f, -2.0f));

        SendConnectPacket();

        return;
    }

    private void SendConnectPacket()
    {
        ConnectData data = new ConnectData();
        data.id = mMyID;
        data.posX = mPlayerTransform.position.x;
        data.posY = mPlayerTransform.position.y;
        data.posZ = mPlayerTransform.position.z;

        ConnectPacket packet = new ConnectPacket(data);
        SendReliable(packet);

        return;
    }

    private void OnReceiveConnectPacket(byte[] packetData)
    {
        ConnectPacket packet = new ConnectPacket(packetData);
        ConnectData data = packet.GetPacketData();

        PlayerInfo info = new PlayerInfo();
        info.id = data.id;

        if (info.id != mMyID)
        {
            Debug.Log("OnReceiveCpnnectPacket :: " + info.id + "client connect !");
            info.pos.x = data.posX;
            info.pos.y = data.posY;
            info.pos.z = data.posZ;

            CreatePlayer(info.id, info.pos);
        }

        return;
    }

    private void OnReceiveDisconnectPacket(byte[] packetData)
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
                // ToDo : 지금은 인스턴스의 컴포넌트에 직접 접근하여 패킷정보를 전달하지만 후에 최적화로 넘어갈 시 분할 하도록 한다.
                DestroyObject(mOtherClients[index].characterInstance);
                mOtherClients.RemoveAt(index);
            }
        }

        return;
    }

    public void SendPlayerMovePacket(Vector3 dir, float h, float v, bool s, Vector3 playerPosition)
    {
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

        if (!mPreviousSavedData.Check(data))
        {
            PlayerMovePacket packet = new PlayerMovePacket(data);
            SendReliable(packet);
        }
        mPreviousSavedData.SetData(data);

        return;
    }

    private void OnReceivePlayerMovePacket(byte[] packetData)
    {
        PlayerMovePacket packet = new PlayerMovePacket(packetData);
        PlayerMoveData data = packet.GetPacketData();

        Vector3 dir = Vector3.zero;
        float h, v;
        bool s;
        PlayerInfo info = new PlayerInfo();

        info.id = data.id;
        info.pos.x = data.posX;
        info.pos.y = data.posY;
        info.pos.z = data.posZ;
        dir.x = data.dirX;
        dir.y = data.dirY;
        dir.z = data.dirZ;
        h = data.horizental;
        v = data.vertical;
        s = data.sneak;

        if (info.id != mMyID)
        {
            int index = mOtherClients.FindIndex(
                    delegate (OtherClientInfo i)
                    {
                        return i.id == info.id;
                    });
            if(-1 != index)
                // ToDo : 지금은 인스턴스의 컴포넌트에 직접 접근하여 패킷정보를 전달하지만 후에 최적화로 넘어갈 시 분할 하도록 한다.
                mOtherClients[index].characterInstance.transform.GetComponent<PlayerMovement>().SetArgument(dir, h, v, s,info.pos);
                // !!!!
                //mOtherClients[index].characterInstance.transform.GetComponent<PlayerMovement>().SetArgument(dir, h, v, s, info.pos, lightOn, lightRotation);
            
        }        
        
        return;
    }

    public void SendPlayerLightPacket(bool on, Quaternion rotation)
    {
        PlayerLightData data = new PlayerLightData();
        data.id = mMyID;
        data.on = on;
        data.rotX = rotation.x;
        data.rotY = rotation.y;
        data.rotZ = rotation.z;
        data.rotW = rotation.w;

        if (!mPreviousSavedData.Check(data))
        {
            PlayerLightPacket packet = new PlayerLightPacket(data);
            SendReliable(packet);
        }
        mPreviousSavedData.SetData(data);

        return;
    }

    private void OnReceivePlayerLightPacket(byte[] packetData)
    {
        PlayerLightPacket packet = new PlayerLightPacket(packetData);
        PlayerLightData data = packet.GetPacketData();
        int id;
        bool on;
        Quaternion rotation;

        id = data.id;
        on = data.on;
        rotation.x = data.rotX;
        rotation.y = data.rotY;
        rotation.z = data.rotZ;
        rotation.w = data.rotW;

        if (id != mMyID)
        {
            int index = mOtherClients.FindIndex(
                    delegate (OtherClientInfo i)
                    {
                        return i.id == id;
                    });
            if (-1 != index)
                // ToDo : 지금은 인스턴스의 컴포넌트에 직접 접근하여 패킷정보를 전달하지만 후에 최적화로 넘어갈 시 분할 하도록 한다.
                mOtherClients[index].characterInstance.transform.GetComponent<PlayerMovement>().SetArgumentLight(on,rotation);
        }
        return;
    }

    private void OnReceiveMonsterSetInfo(byte[] packetData)
    {
        MonsterSetInfoPacket packet = new MonsterSetInfoPacket(packetData);
        MonsterSetInfoData data = packet.GetPacketData();

        mMonsterCurPos.x = data.posX;
        mMonsterCurPos.y = data.posY;
        mMonsterCurPos.z = data.posZ;
        mMonsterPatrolPos.x = data.patrolPosX;
        mMonsterPatrolPos.y = data.patrolPosY;
        mMonsterPatrolPos.z = data.patrolPosZ;

        enemy.GetComponent<EnemyAI>().SetArgument(mMonsterPatrolPos);
        enemy.GetComponent<NavMeshAgent>().Warp(mMonsterCurPos);
        enemy.GetComponent<EnemyAI>().connectNetwork = true;

        Debug.Log("enter!!!! MonsterReceive!!!");
        return;
    }

    public void SendMonsterMovePacket(Vector3 pos)
    {
        MonsterMoveData data = new MonsterMoveData();
        data.posX = pos.x;
        data.posY = pos.y;
        data.posZ = pos.z;
        
        if (!mPreviousSavedData.Check(data))
        {
            MonsterMovePacket packet = new MonsterMovePacket(data);
            SendReliable(packet);
        }
        mPreviousSavedData.SetData(data);

        return;
    }

    public void SendPlayerShoutPacket(bool shouting, Vector3 position)
    {
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
    
    private void OnReceivePlayerShoutPacket(byte[] packetData)
    {
        PlayerShoutPacket packet = new PlayerShoutPacket(packetData);
        PlayerShoutData data = packet.GetPacketData();

        int id;
        bool shoutBool;
        Vector3 otherPlayerPosition;

        id = data.id;
        shoutBool = data.shouting;
        otherPlayerPosition.x = data.posX;
        otherPlayerPosition.y = data.posY;
        otherPlayerPosition.z = data.posZ;

        if (id != mMyID)
        {
            int index = mOtherClients.FindIndex(
                    delegate (OtherClientInfo i)
                    {
                        return i.id == id;
                    });
            if (-1 != index)
                // ToDo : 지금은 인스턴스의 컴포넌트에 직접 접근하여 패킷정보를 전달하지만 후에 최적화로 넘어갈 시 분할 하도록 한다.
                mOtherClients[index].characterInstance.transform.GetComponent<PlayerMovement>().SetArgumentShout(shoutBool, otherPlayerPosition);
        }
        return;
    }

    public Vector3 GetMonsterPosition()
    {
        return mMonsterCurPos;
    }

    //***************************************************************************************

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

    //***************************************************************************************

    // ToDo : 이를 활용한 다른 컴포넌트에 패킷 정보를 전달하는 부분을 보완해야 할 것.
    /*
    // 수신 패킷 처리함수 델리게이트. 
    public delegate void RecvNotifier(PacketID id, byte[] data);
    // 수신 패킷 분배 해시 테이블.
    private Dictionary<int, RecvNotifier> m_notifier = new Dictionary<int, RecvNotifier>();
    */

        /*
    public void RegisterReceiveNotification(PacketID id, RecvNotifier notifier)
    {
        int index = (int)id;

        if (m_notifier.ContainsKey(index))
        {
            m_notifier.Remove(index);
        }

        m_notifier.Add(index, notifier);
    }

    public void UnregisterReceiveNotification(PacketID id)
    {
        int index = (int)id;

        if (m_notifier.ContainsKey(index))
        {
            m_notifier.Remove(index);
        }
    }
    */
}