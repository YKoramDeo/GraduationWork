using UnityEngine;
using System.Net.Sockets;

public enum NetConfig
{
    MAX_PLAYER = 4,
    SERVER_PORT = 4000,
    MAX_BUFFER_SIZE = 1400,
    NIL = -9999
}

public enum NetEventType
{
    Connect = 0,
    Disconnect,
    SendError,
    ReceiveError
}

public enum NetEventResult
{
    Failure = -1,
    Success = 0
}

public class NetEventState
{
    public NetEventType mType;
    public NetEventResult mResult;
}

public struct PlayerInfo
{
    public int id;
    public Vector3 pos;
    public Quaternion rot;
    public bool lightOn;
    public Quaternion lightRotation;
    public Socket socket;
}

public interface IPacket<T>
{
    // 
    byte GetPacketType();

    //	
    T GetPacketData();

    //
    byte[] GetByteData();
}

public enum PacketType
{
    SetID,
    Connect,
    Disconnect,
    PlayerMove,
    PlayerLight,
    PlayerShout,
    MonsterSetInfo,
    MonsterMove,
    MonsterSetPatrolPos
}

public struct PacketHeader
{
    public byte size;
    public byte type;
}

public struct SetIDData
{
    public int id;
}

public struct ConnectData
{
    public int id;
    public float posX;
    public float posY;
    public float posZ;
}

public struct DisconnectData
{
    public int id;
}

public struct PlayerMoveData
{
    public int id;
    public float posX;
    public float posY;
    public float posZ;

    public float dirX;
    public float dirY;
    public float dirZ;

    public float horizental;
    public float vertical;
    public bool sneak;
}

public struct PlayerLightData
{
    public int id;
    public bool on;
    public float rotX;
    public float rotY;
    public float rotZ;
    public float rotW;
}

public struct PlayerShoutData
{
    public int id;
    public bool shouting;
    public float posX;
    public float posY;
    public float posZ;
}

public struct MonsterSetInfoData
{
    public float posX;
    public float posY;
    public float posZ;
    public float patrolPosX;
    public float patrolPosY;
    public float patrolPosZ;
};

public struct MonsterMoveData
{
    public float posX;
    public float posY;
    public float posZ;
};

public struct MonsterSetPatrolPosData
{
    public float posX;
    public float posY;
    public float posZ;
};
