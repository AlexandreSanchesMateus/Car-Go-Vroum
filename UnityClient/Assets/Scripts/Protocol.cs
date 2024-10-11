using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net;
using System.Text;
using ENet6;
using Unity.VisualScripting;
using System.Reflection.Emit;


namespace NetworkProtocol
{
    public enum EOpcode : byte
    {
        C_PlayerName,
        C_PlayerReady,
        C_PlayerInputs,

        S_GameData,
        S_PlayerConnected,
        S_PlayerDisconnected,
        S_Ready,
    }

    public enum DisconnectReport : UInt32
    {
        DISCONNECTED,
        SERVER_END,
	    KICK,
	    LOBBY_FULL,
	    GAME_LAUNCHED
    };

    public abstract class BasePacket
    {
        public EOpcode Opcode { get; protected set; }
        public abstract void Serialize(List<byte> byteArray);
    }

    public class PlayerNamePacket : BasePacket
    {
        public PlayerNamePacket() { Opcode = EOpcode.C_PlayerName; }

        public string name;

        public override void Serialize(List<byte> byteArray)
        {
            Serializer.Serialize_str(byteArray, name);
        }

        public static PlayerNamePacket Deserialize(List<byte> byteArray, ref int offset)
        {
            PlayerNamePacket packet = new PlayerNamePacket();
            packet.name = Serializer.Deserialize_str(byteArray, ref offset);

            return packet;
        }
    }

    public class PlayerReadyPacket : BasePacket
    {
        public PlayerReadyPacket() { Opcode = EOpcode.C_PlayerReady; }

        public bool ready;

        public override void Serialize(List<byte> byteArray)
        {
            byte octet = (byte)(ready ? 1 : 0);
            Serializer.Serialize_uByte(byteArray, octet);
        }

        public static PlayerReadyPacket Deserialize(List<byte> byteArray, ref int offset)
        {
            PlayerReadyPacket packet = new PlayerReadyPacket();
            packet.ready = Serializer.Deserialize_uByte(byteArray, ref offset) == 1;

            return packet;
        }
    }

    public class PlayerInputPacket : BasePacket
    {
        public PlayerInputPacket() { Opcode = EOpcode.C_PlayerInputs; }

        public PlayerInput inputs = new PlayerInput();

        public override void Serialize(List<byte> byteArray)
        {
            Serializer.Serialize_float(byteArray, inputs.acceleration);
            Serializer.Serialize_float(byteArray, inputs.steer);

            byte octet = (byte)(inputs.brake ? 1 : 0);
            Serializer.Serialize_uByte(byteArray, octet);
        }

        public static PlayerInputPacket Deserialize(List<byte> byteArray, ref int offset)
        {
            PlayerInputPacket packet = new PlayerInputPacket();

            packet.inputs.acceleration = Serializer.Deserialize_float(byteArray, ref offset);
            packet.inputs.steer = Serializer.Deserialize_float(byteArray, ref offset);
            packet.inputs.brake = Serializer.Deserialize_uByte(byteArray, ref offset) == 1;

            return packet;
        }
    }

    public class PlayerConnectPacket : BasePacket
    {
        public PlayerConnectPacket() {  Opcode = EOpcode.S_PlayerConnected; }

        public UInt16 playerIndex;
        public string name;
        public bool ready;

        public override void Serialize(List<byte> byteArray)
        {
            Serializer.Serialize_u16(byteArray, playerIndex);
            Serializer.Serialize_str(byteArray, name);
            Serializer.Serialize_uByte(byteArray, (byte)(ready ? 1 : 0));
        }

        public static PlayerConnectPacket Deserialize(List<byte> byteArray, ref int offset)
        {
            PlayerConnectPacket packet = new PlayerConnectPacket();
            packet.playerIndex = Serializer.Deserialize_u16(byteArray, ref offset);
            packet.name = Serializer.Deserialize_str(byteArray, ref offset);
            packet.ready = Serializer.Deserialize_uByte(byteArray, ref offset) == 1;

            return packet;
        }
    }

    public class PlayerDisconnectedPacket : BasePacket
    {
        public PlayerDisconnectedPacket() { Opcode= EOpcode.S_PlayerDisconnected; }

        public UInt16 playerIndex;

        public override void Serialize(List<byte> byteArray)
        {
            Serializer.Serialize_u16(byteArray, playerIndex);
        }

        public static PlayerDisconnectedPacket Deserialize(List<byte> byteArray, ref int offset)
        {
            PlayerDisconnectedPacket packet = new PlayerDisconnectedPacket();
            packet.playerIndex = Serializer.Deserialize_u16(byteArray, ref offset);

            return packet;
        }
    }

    public class ReadyPacket : BasePacket
    {
        public ReadyPacket() { Opcode= EOpcode.S_Ready;}

        public UInt16 playerIndex;
        public bool ready;

        public override void Serialize(List<byte> byteArray)
        {
            Serializer.Serialize_u16(byteArray, playerIndex);
            Serializer.Serialize_uByte(byteArray, (byte)(ready ? 1 : 0));
        }

        public static ReadyPacket Deserialize(List<byte> byteArray, ref int offset)
        {
            ReadyPacket packet = new ReadyPacket();
            packet.playerIndex = Serializer.Deserialize_u16(byteArray, ref offset);
            packet.ready = Serializer.Deserialize_uByte(byteArray, ref offset) == 1;

            return packet;
        }
    }

    public class GameDataPacket : BasePacket
    {
        public GameDataPacket() { Opcode = EOpcode.S_GameData; }

        public struct PlayerPacketData
        {
            public UInt16 index;
            public string name;
            public bool ready;
        };

        public UInt16 targetPlayerIndex;
        public List<PlayerPacketData> playerList = new List<PlayerPacketData>();

        public override void Serialize(List<byte> byteArray)
        {
            Serializer.Serialize_u16(byteArray, targetPlayerIndex);
            Serializer.Serialize_uByte(byteArray, (byte)playerList.Count);

            foreach (PlayerPacketData data in playerList)
            {
                Serializer.Serialize_u16(byteArray, data.index);
                Serializer.Serialize_str(byteArray, data.name);
                byte octet = (byte)(data.ready ? 1 : 0);
                Serializer.Serialize_uByte(byteArray, octet);
            }
        }

        public static GameDataPacket Deserialize(List<byte> byteArray, ref int offset)
        {
            GameDataPacket packet = new GameDataPacket();
            packet.targetPlayerIndex = Serializer.Deserialize_u16(byteArray, ref offset);

            int count = Serializer.Deserialize_uByte(byteArray, ref offset);
            for (int i = 0; i < count; ++i)
            {
                PlayerPacketData player;
                player.index = Serializer.Deserialize_u16(byteArray, ref offset);
                player.name = Serializer.Deserialize_str(byteArray, ref offset);
                player.ready = Serializer.Deserialize_uByte(byteArray, ref offset) == 1;

                packet.playerList.Add(player);
            }

            return packet;
        }
    }

    public class Serializer
    {
        public static void Serialize_str(List<byte> byteArray, string str)
        {
            if (!string.IsNullOrEmpty(str))
            {
                Serialize_u32(byteArray, (UInt32)str.Length);
                byteArray.AddRange(Encoding.UTF8.GetBytes(str));
            }
            else
                Serialize_u32(byteArray, 0);
        }

        public static void Serialize_float(List<byte> byteArray, float value)
        {
            Serialize_i32(byteArray, BitConverter.SingleToInt32Bits(value));
        }

        public static void Serialize_sByte(List<byte> byteArray, sbyte value)
        {
            Serialize_uByte(byteArray, (byte)value);
        }

        public static void Serialize_i16(List<byte> byteArray, Int16 value)
        {
            byte[] networkValue = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(value));
            byteArray.AddRange(networkValue);
        }

        public static void Serialize_i32(List<byte> byteArray, Int32 value)
        {
            byte[] networkValue = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(value));
            byteArray.AddRange(networkValue);
        }

        public static void Serialize_uByte(List<byte> byteArray, byte value)
        {
            byteArray.Add(value);
        }

        public static void Serialize_u16(List<byte> byteArray, UInt16 value)
        {
            Serialize_i16(byteArray, (Int16)value);
        }

        public static void Serialize_u32(List<byte> byteArray, UInt32 value)
        {
            Serialize_i32(byteArray, (Int32)value);
        }

        public static string Deserialize_str(List<byte> byteArray, ref int offset)
        {
            int strSize = (Int32)Deserialize_u32(byteArray, ref offset);

            if(strSize != 0)
            {
                string str = Encoding.UTF8.GetString(byteArray.ToArray(), offset, strSize);
                offset += strSize;
                return str;
            }

            return "";
        }

        public static float Deserialize_float(List<byte> byteArray, ref int offset)
        {
            return BitConverter.Int32BitsToSingle(Deserialize_i32(byteArray, ref offset));
        }

        public static sbyte Deserialize_sByte(List<byte> byteArray, ref int offset)
        {
            return (sbyte)Deserialize_uByte(byteArray, ref offset);
        }

        public static Int16 Deserialize_i16(List<byte> byteArray, ref int offset)
        {
            Int16 value = BitConverter.ToInt16(byteArray.ToArray(), offset);
            offset += sizeof(Int16);

            return (Int16)IPAddress.NetworkToHostOrder(value);
        }

        public static Int32 Deserialize_i32(List<byte> byteArray, ref int offset)
        {
            Int32 value = BitConverter.ToInt32(byteArray.ToArray(), offset);
            offset += sizeof(Int32);

            return (Int32)IPAddress.NetworkToHostOrder(value);
        }

        public static byte Deserialize_uByte(List<byte> byteArray, ref int offset)
        {
            byte value = byteArray[offset];
            offset += sizeof(byte);

            return value;
        }

        public static UInt16 Deserialize_u16(List<byte> byteArray, ref int offset)
        {
            UInt16 value = BitConverter.ToUInt16(byteArray.ToArray(), offset);
            offset += sizeof(UInt16);

            return (UInt16)IPAddress.NetworkToHostOrder((Int16)value);
        }

        public static UInt32 Deserialize_u32(List<byte> byteArray, ref int offset)
        {
            UInt32 value = BitConverter.ToUInt32(byteArray.ToArray(), offset);
            offset += sizeof(UInt32);

            return (UInt32)IPAddress.NetworkToHostOrder((Int32)value);
        }
    }
}
