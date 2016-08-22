using UnityEngine;
using System.Collections;

public class HeaderSerializer : Serializer
{
    public bool Serialize(PacketHeader header)
    {
        // 기존 데이터를 클리어합니다.
        Clear();

        // 각 요소를 사례로 시리얼라이즈합니다.
        bool retval = true;
        retval &= Serialize(header.size);
        retval &= Serialize(header.type);

        if (retval == false)
        {
            return false;
        }

        return true;
    }

    public bool Deserialize(byte[] data, ref PacketHeader header)
    {
        // 디시리얼라이즈할 데이터를 설정합니다.
        bool retval = SetDeserializedData(data);
        if (retval == false)
        {
            return false;
        }

        // 데이터의 요소별로 디시리얼라이즈합니다.
        byte type = 0;
        byte size = 0;
        retval &= Deserialize(ref size);
        retval &= Deserialize(ref type);

        header.size = size;     
        header.type = type;

        return retval;
    }
}

public class SetIDPacket : IPacket<SetIDData>
{
    private class SetIDSerializer : Serializer
    {
        public bool Serialize(SetIDData data)
        {
            bool retval = true;

            retval &= Serialize(data.id);

            return retval;
        }

        public bool Deserialize(ref SetIDData data)
        {
            if (GetDataSize() == 0) return false;

            bool retval = true;

            retval &= Deserialize(ref data.id);

            return retval;
        }
    }

    // 패킷 데이터의 실체
    SetIDData mPacketData;

    public SetIDPacket(SetIDData data)
    {
        mPacketData = data;
    }

    public SetIDPacket(byte[] data)
    {
        SetIDSerializer serializer = new SetIDSerializer();

        serializer.SetDeserializedData(data);
        serializer.Deserialize(ref mPacketData);
    }

    public byte GetPacketType()
    {
        return (byte)PacketType.SetID;
    }

    public SetIDData GetPacketData()
    {
        return mPacketData;
    }

    public byte[] GetByteData()
    {
        SetIDSerializer serializer = new SetIDSerializer();
        serializer.Serialize(mPacketData);
        return serializer.GetSerializedData();
    }
}

public class NotifyPacket : IPacket<NotifyData>
{
    private class NotifySerializer : Serializer
    {
        public bool Serialize(NotifyData data)
        {
            bool retval = true;

            retval &= Serialize(data.id);
            retval &= Serialize(data.notice);

            return retval;
        }

        public bool Deserialize(ref NotifyData data)
        {
            if (GetDataSize() == 0) return false;

            bool retval = true;

            retval &= Deserialize(ref data.id);
            retval &= Deserialize(ref data.notice);

            return retval;
        }
    }

    // 패킷 데이터의 실체
    NotifyData mPacketData;

    public NotifyPacket(NotifyData data)
    {
        mPacketData = data;

    }

    public NotifyPacket(byte[] data)
    {
        NotifySerializer serializer = new NotifySerializer();

        serializer.SetDeserializedData(data);
        serializer.Deserialize(ref mPacketData);
    }

    public byte GetPacketType()
    {
        return (byte)PacketType.Notify;
    }

    public NotifyData GetPacketData()
    {
        return mPacketData;
    }

    public byte[] GetByteData()
    {
        NotifySerializer serializer = new NotifySerializer();
        serializer.Serialize(mPacketData);
        return serializer.GetSerializedData();
    }
}

public class DisconnectPacket : IPacket<DisconnectData>
{
    private class DisconnectSerializer : Serializer
    {
        public bool Serialize(DisconnectData data)
        {
            bool retval = true;

            retval &= Serialize(data.id);
            
            return retval;
        }

        public bool Deserialize(ref DisconnectData data)
        {
            if (GetDataSize() == 0) return false;

            bool retval = true;

            retval &= Deserialize(ref data.id);
            
            return retval;
        }
    }

    // 패킷 데이터의 실체
    DisconnectData mPacketData;

    public DisconnectPacket(DisconnectData data)
    {
        mPacketData = data;
    }

    public DisconnectPacket(byte[] data)
    {
        DisconnectSerializer serializer = new DisconnectSerializer();

        serializer.SetDeserializedData(data);
        serializer.Deserialize(ref mPacketData);
    }

    public byte GetPacketType()
    {
        return (byte)PacketType.Disconnect;
    }

    public DisconnectData GetPacketData()
    {
        return mPacketData;
    }

    public byte[] GetByteData()
    {
        DisconnectSerializer serializer = new DisconnectSerializer();
        serializer.Serialize(mPacketData);
        return serializer.GetSerializedData();
    }
}

public class CreateRoomPacket : IPacket<CreateRoomData>
{
    private class CreateRoomSerializer : Serializer
    {
        public bool Serialize(CreateRoomData data)
        {
            bool retval = true;

            retval &= Serialize(data.roomNo);
            retval &= Serialize(data.chiefNo);
            retval &= Serialize(data.partner_1_ID);
            retval &= Serialize(data.partner_2_ID);
            retval &= Serialize(data.partner_3_ID);

            return retval;
        }

        public bool Deserialize(ref CreateRoomData data)
        {
            if (GetDataSize() == 0) return false;

            bool retval = true;

            retval &= Deserialize(ref data.roomNo);
            retval &= Deserialize(ref data.chiefNo);
            retval &= Deserialize(ref data.partner_1_ID);
            retval &= Deserialize(ref data.partner_2_ID);
            retval &= Deserialize(ref data.partner_3_ID);

            return retval;
        }
    }

    // 패킷 데이터의 실체
    CreateRoomData mPacketData;

    public CreateRoomPacket(CreateRoomData data)
    {
        mPacketData = data;

    }

    public CreateRoomPacket(byte[] data)
    {
        CreateRoomSerializer serializer = new CreateRoomSerializer();

        serializer.SetDeserializedData(data);
        serializer.Deserialize(ref mPacketData);
    }

    public byte GetPacketType()
    {
        return (byte)PacketType.CreateRoom;
    }

    public CreateRoomData GetPacketData()
    {
        return mPacketData;
    }

    public byte[] GetByteData()
    {
        CreateRoomSerializer serializer = new CreateRoomSerializer();
        serializer.Serialize(mPacketData);
        return serializer.GetSerializedData();
    }
}

public class JoinRoomPacket : IPacket<JoinRoomData>
{
    private class JoinRoomSerializer : Serializer
    {
        public bool Serialize(JoinRoomData data)
        {
            bool retval = true;

            retval &= Serialize(data.roomNo);

            return retval;
        }

        public bool Deserialize(ref JoinRoomData data)
        {
            if (GetDataSize() == 0) return false;

            bool retval = true;
            retval &= Deserialize(ref data.roomNo);

            return retval;
        }
    }

    // 패킷 데이터의 실체
    JoinRoomData mPacketData;

    public JoinRoomPacket(JoinRoomData data)
    {
        mPacketData = data;

    }

    public JoinRoomPacket(byte[] data)
    {
        JoinRoomSerializer serializer = new JoinRoomSerializer();

        serializer.SetDeserializedData(data);
        serializer.Deserialize(ref mPacketData);
    }

    public byte GetPacketType()
    {
        return (byte)PacketType.JoinRoom;
    }

    public JoinRoomData GetPacketData()
    {
        return mPacketData;
    }

    public byte[] GetByteData()
    {
        JoinRoomSerializer serializer = new JoinRoomSerializer();
        serializer.Serialize(mPacketData);
        return serializer.GetSerializedData();
    }
}

public class RenewalRoomInfoPacket : IPacket<RenewalRoomInfoData>
{
    private class RenewalRoomInfoSerializer : Serializer
    {
        public bool Serialize(RenewalRoomInfoData data)
        {
            bool retval = true;

            retval &= Serialize(data.roomNo);
            retval &= Serialize(data.chiefNo);
            retval &= Serialize(data.partner_1_ID);
            retval &= Serialize(data.partner_1_ready);
            retval &= Serialize(data.partner_2_ID);
            retval &= Serialize(data.partner_2_ready);
            retval &= Serialize(data.partner_3_ID);
            retval &= Serialize(data.partner_3_ready);

            return retval;
        }

        public bool Deserialize(ref RenewalRoomInfoData data)
        {
            if (GetDataSize() == 0) return false;

            bool retval = true;

            retval &= Deserialize(ref data.roomNo);
            retval &= Deserialize(ref data.chiefNo);
            retval &= Deserialize(ref data.partner_1_ID);
            retval &= Deserialize(ref data.partner_1_ready);
            retval &= Deserialize(ref data.partner_2_ID);
            retval &= Deserialize(ref data.partner_2_ready);
            retval &= Deserialize(ref data.partner_3_ID);
            retval &= Deserialize(ref data.partner_3_ready);

            return retval;
        }
    }

    // 패킷 데이터의 실체
    RenewalRoomInfoData mPacketData;

    public RenewalRoomInfoPacket(RenewalRoomInfoData data)
    {
        mPacketData = data;
    }

    public RenewalRoomInfoPacket(byte[] data)
    {
        RenewalRoomInfoSerializer serializer = new RenewalRoomInfoSerializer();

        serializer.SetDeserializedData(data);
        serializer.Deserialize(ref mPacketData);
    }

    public byte GetPacketType()
    {
        return (byte)PacketType.RenewalRoomInfo;
    }

    public RenewalRoomInfoData GetPacketData()
    {
        return mPacketData;
    }

    public byte[] GetByteData()
    {
        RenewalRoomInfoSerializer serializer = new RenewalRoomInfoSerializer();
        serializer.Serialize(mPacketData);
        return serializer.GetSerializedData();
    }
}

/***************************************************/
public class ConnectPacket : IPacket<ConnectData>
{
    private class ConnectSerializer : Serializer
    {
        public bool Serialize(ConnectData data)
        {
            bool retval = true;

            retval &= Serialize(data.id);
            retval &= Serialize(data.posX);
            retval &= Serialize(data.posY);
            retval &= Serialize(data.posZ);

            return retval;
        }

        public bool Deserialize(ref ConnectData data)
        {
            if (GetDataSize() == 0) return false;

            bool retval = true;
            retval &= Deserialize(ref data.id);
            retval &= Deserialize(ref data.posX);
            retval &= Deserialize(ref data.posY);
            retval &= Deserialize(ref data.posZ);

            return retval;
        }
    }

    // 패킷 데이터의 실체
    ConnectData mPacketData;

    public ConnectPacket(ConnectData data)
    {
        mPacketData = data;
    }

    public ConnectPacket(byte[] data)
    {
        ConnectSerializer serializer = new ConnectSerializer();

        serializer.SetDeserializedData(data);
        serializer.Deserialize(ref mPacketData);
    }

    public byte GetPacketType()
    {
        return (byte)PacketType.Connect;
    }

    public ConnectData GetPacketData()
    {
        return mPacketData;
    }

    public byte[] GetByteData()
    {
        ConnectSerializer serializer = new ConnectSerializer();
        serializer.Serialize(mPacketData);
        return serializer.GetSerializedData();
    }
}


public class PlayerMovePacket : IPacket<PlayerMoveData>
{
    private class PlayerMoveSerializer : Serializer
    {
        public bool Serialize(PlayerMoveData data)
        {
            bool retval = true;

            retval &= Serialize(data.id);
            retval &= Serialize(data.posX);
            retval &= Serialize(data.posY);
            retval &= Serialize(data.posZ);
            retval &= Serialize(data.dirX);
            retval &= Serialize(data.dirY);
            retval &= Serialize(data.dirZ);
            retval &= Serialize(data.horizental);
            retval &= Serialize(data.vertical);
            retval &= Serialize(data.sneak);
            return retval;
        }

        public bool Deserialize(ref PlayerMoveData data)
        {
            if (GetDataSize() == 0) return false;

            bool retval = true;
            retval &= Deserialize(ref data.id);
            retval &= Deserialize(ref data.posX);
            retval &= Deserialize(ref data.posY);
            retval &= Deserialize(ref data.posZ);
            retval &= Deserialize(ref data.dirX);
            retval &= Deserialize(ref data.dirY);
            retval &= Deserialize(ref data.dirZ);
            retval &= Deserialize(ref data.horizental);
            retval &= Deserialize(ref data.vertical);
            retval &= Deserialize(ref data.sneak);
            return retval;
        }
    }

    // 패킷 데이터의 실체
    PlayerMoveData mPacketData;

    public PlayerMovePacket(PlayerMoveData data)
    {
        mPacketData = data;
    }

    public PlayerMovePacket(byte[] data)
    {
        PlayerMoveSerializer serializer = new PlayerMoveSerializer();

        serializer.SetDeserializedData(data);
        serializer.Deserialize(ref mPacketData);
    }

    public byte GetPacketType()
    {
        return (byte)PacketType.PlayerMove;
    }

    public PlayerMoveData GetPacketData()
    {
        return mPacketData;
    }

    public byte[] GetByteData()
    {
        PlayerMoveSerializer serializer = new PlayerMoveSerializer();
        serializer.Serialize(mPacketData);
        return serializer.GetSerializedData();
    }
}

public class PlayerLightPacket : IPacket<PlayerLightData>
{
    private class PlayerLightSerializer : Serializer
    {
        public bool Serialize(PlayerLightData data)
        {
            bool retval = true;

            retval &= Serialize(data.id);
            retval &= Serialize(data.on);
            retval &= Serialize(data.rotX);
            retval &= Serialize(data.rotY);
            retval &= Serialize(data.rotZ);
            retval &= Serialize(data.rotW);

            return retval;
        }

        public bool Deserialize(ref PlayerLightData data)
        {
            if (GetDataSize() == 0) return false;

            bool retval = true;
            retval &= Deserialize(ref data.id);
            retval &= Deserialize(ref data.on);
            retval &= Deserialize(ref data.rotX);
            retval &= Deserialize(ref data.rotY);
            retval &= Deserialize(ref data.rotZ);
            retval &= Deserialize(ref data.rotW);
            return retval;
        }
    }

    // 패킷 데이터의 실체
    PlayerLightData mPacketData;

    public PlayerLightPacket(PlayerLightData data)
    {
        mPacketData = data;
    }

    public PlayerLightPacket(byte[] data)
    {
        PlayerLightSerializer serializer = new PlayerLightSerializer();

        serializer.SetDeserializedData(data);
        serializer.Deserialize(ref mPacketData);
    }

    public byte GetPacketType()
    {
        return (byte)PacketType.PlayerLight;
    }

    public PlayerLightData GetPacketData()
    {
        return mPacketData;
    }

    public byte[] GetByteData()
    {
        PlayerLightSerializer serializer = new PlayerLightSerializer();
        serializer.Serialize(mPacketData);
        return serializer.GetSerializedData();
    }
}

public class PlayerShoutPacket : IPacket<PlayerShoutData>
{
    private class PlayerShoutSerializer : Serializer
    {
        public bool Serialize(PlayerShoutData data)
        {
            bool retval = true;

            retval &= Serialize(data.id);
            retval &= Serialize(data.shouting);
            retval &= Serialize(data.posX);
            retval &= Serialize(data.posY);
            retval &= Serialize(data.posZ);

            return retval;
        }

        public bool Deserialize(ref PlayerShoutData data)
        {
            if (GetDataSize() == 0) return false;

            bool retval = true;
            retval &= Deserialize(ref data.id);
            retval &= Deserialize(ref data.shouting);
            retval &= Deserialize(ref data.posX);
            retval &= Deserialize(ref data.posY);
            retval &= Deserialize(ref data.posZ);

            return retval;
        }
    }

    // 패킷 데이터의 실체
    PlayerShoutData mPacketData;

    public PlayerShoutPacket(PlayerShoutData data)
    {
        mPacketData = data;
    }

    public PlayerShoutPacket(byte[] data)
    {
        PlayerShoutSerializer serializer = new PlayerShoutSerializer();

        serializer.SetDeserializedData(data);
        serializer.Deserialize(ref mPacketData);
    }

    public byte GetPacketType()
    {
        return (byte)PacketType.PlayerShout;
    }

    public PlayerShoutData GetPacketData()
    {
        return mPacketData;
    }

    public byte[] GetByteData()
    {
        PlayerShoutSerializer serializer = new PlayerShoutSerializer();
        serializer.Serialize(mPacketData);
        return serializer.GetSerializedData();
    }
}

public class PlayerGetItemPacket : IPacket<PlayerGetItemData>
{
    private class PlayerGetItemSerializer : Serializer
    {
        public bool Serialize(PlayerGetItemData data)
        {
            bool retval = true;

            retval &= Serialize(data.id);
            retval &= Serialize(data.itemID);

            return retval;
        }

        public bool Deserialize(ref PlayerGetItemData data)
        {
            if (GetDataSize() == 0) return false;

            bool retval = true;
            retval &= Deserialize(ref data.id);
            retval &= Deserialize(ref data.itemID);

            return retval;
        }
    }

    // 패킷 데이터의 실체
    PlayerGetItemData mPacketData;

    public PlayerGetItemPacket(PlayerGetItemData data)
    {
        mPacketData = data;
    }

    public PlayerGetItemPacket(byte[] data)
    {
        PlayerGetItemSerializer serializer = new PlayerGetItemSerializer();

        serializer.SetDeserializedData(data);
        serializer.Deserialize(ref mPacketData);
    }

    public byte GetPacketType()
    {
        return (byte)PacketType.PlayerGetItem;
    }

    public PlayerGetItemData GetPacketData()
    {
        return mPacketData;
    }

    public byte[] GetByteData()
    {
        PlayerGetItemSerializer serializer = new PlayerGetItemSerializer();
        serializer.Serialize(mPacketData);
        return serializer.GetSerializedData();
    }
}

public class MonsterSetInfoPacket : IPacket<MonsterSetInfoData>
{
    private class MonsterSetInfoSerializer : Serializer
    {
        public bool Serialize(MonsterSetInfoData data)
        {
            bool retval = true;

            retval &= Serialize(data.posX);
            retval &= Serialize(data.posY);
            retval &= Serialize(data.posZ);
            retval &= Serialize(data.patrolPosX);
            retval &= Serialize(data.patrolPosY);
            retval &= Serialize(data.patrolPosZ);

            return retval;
        }

        public bool Deserialize(ref MonsterSetInfoData data)
        {
            if (GetDataSize() == 0) return false;

            bool retval = true;
            retval &= Deserialize(ref data.posX);
            retval &= Deserialize(ref data.posY);
            retval &= Deserialize(ref data.posZ);
            retval &= Deserialize(ref data.patrolPosX);
            retval &= Deserialize(ref data.patrolPosY);
            retval &= Deserialize(ref data.patrolPosZ);

            return retval;
        }
    }

    // 패킷 데이터의 실체
    MonsterSetInfoData mPacketData;

    public MonsterSetInfoPacket(MonsterSetInfoData data)
    {
        mPacketData = data;
    }

    public MonsterSetInfoPacket(byte[] data)
    {
        MonsterSetInfoSerializer serializer = new MonsterSetInfoSerializer();

        serializer.SetDeserializedData(data);
        serializer.Deserialize(ref mPacketData);
    }

    public byte GetPacketType()
    {
        return (byte)PacketType.MonsterSetInfo;
    }

    public MonsterSetInfoData GetPacketData()
    {
        return mPacketData;
    }

    public byte[] GetByteData()
    {
        MonsterSetInfoSerializer serializer = new MonsterSetInfoSerializer();
        serializer.Serialize(mPacketData);
        return serializer.GetSerializedData();
    }
}

public class MonsterMovePacket : IPacket<MonsterMoveData>
{
    private class MonsterMoveSerializer : Serializer
    {
        public bool Serialize(MonsterMoveData data)
        {
            bool retval = true;

            retval &= Serialize(data.posX);
            retval &= Serialize(data.posY);
            retval &= Serialize(data.posZ);

            return retval;
        }

        public bool Deserialize(ref MonsterMoveData data)
        {
            if (GetDataSize() == 0) return false;

            bool retval = true;
            retval &= Deserialize(ref data.posX);
            retval &= Deserialize(ref data.posY);
            retval &= Deserialize(ref data.posZ);

            return retval;
        }
    }

    // 패킷 데이터의 실체
    MonsterMoveData mPacketData;

    public MonsterMovePacket(MonsterMoveData data)
    {
        mPacketData = data;
    }

    public MonsterMovePacket(byte[] data)
    {
        MonsterMoveSerializer serializer = new MonsterMoveSerializer();

        serializer.SetDeserializedData(data);
        serializer.Deserialize(ref mPacketData);
    }

    public byte GetPacketType()
    {
        return (byte)PacketType.MonsterMove;
    }

    public MonsterMoveData GetPacketData()
    {
        return mPacketData;
    }

    public byte[] GetByteData()
    {
        MonsterMoveSerializer serializer = new MonsterMoveSerializer();
        serializer.Serialize(mPacketData);
        return serializer.GetSerializedData();
    }
}

public class MonsterSetPatrolPosPacket : IPacket<MonsterSetPatrolPosData>
{
    private class MonsterSetPatrolPosSerializer : Serializer
    {
        public bool Serialize(MonsterSetPatrolPosData data)
        {
            bool retval = true;

            retval &= Serialize(data.posX);
            retval &= Serialize(data.posY);
            retval &= Serialize(data.posZ);

            return retval;
        }

        public bool Deserialize(ref MonsterSetPatrolPosData data)
        {
            if (GetDataSize() == 0) return false;

            bool retval = true;
            retval &= Deserialize(ref data.posX);
            retval &= Deserialize(ref data.posY);
            retval &= Deserialize(ref data.posZ);

            return retval;
        }
    }

    // 패킷 데이터의 실체
    MonsterSetPatrolPosData mPacketData;

    public MonsterSetPatrolPosPacket(MonsterSetPatrolPosData data)
    {
        mPacketData = data;
    }

    public MonsterSetPatrolPosPacket(byte[] data)
    {
        MonsterSetPatrolPosSerializer serializer = new MonsterSetPatrolPosSerializer();

        serializer.SetDeserializedData(data);
        serializer.Deserialize(ref mPacketData);
    }

    public byte GetPacketType()
    {
        return (byte)PacketType.MonsterSetPatrolPos;
    }

    public MonsterSetPatrolPosData GetPacketData()
    {
        return mPacketData;
    }

    public byte[] GetByteData()
    {
        MonsterSetPatrolPosSerializer serializer = new MonsterSetPatrolPosSerializer();
        serializer.Serialize(mPacketData);
        return serializer.GetSerializedData();
    }
}
/***************************************************/
