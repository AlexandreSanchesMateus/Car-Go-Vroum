using System;
using System.Collections.Generic;
using System.Net;
using System.Text;


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

        S_RunningState,
        S_StartMovingState,
        S_StartGameState,
        S_FinishedState,
        S_PlayersState,
        S_PlayerInfected,
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
            packet.ready = Serializer.Deserialize_uByte(byteArray, ref offset) != 0;

            return packet;
        }
    }

    public class PlayerInputPacket : BasePacket
    {
        public PlayerInputPacket() { Opcode = EOpcode.C_PlayerInputs; }

        public UInt16 inputIndex;
        public PlayerInput inputs = new PlayerInput();

        public override void Serialize(List<byte> byteArray)
        {
            Serializer.Serialize_u16(byteArray, inputIndex);
            inputs.Serialize(byteArray);
        }

        public static PlayerInputPacket Deserialize(List<byte> byteArray, ref int offset)
        {
            PlayerInputPacket packet = new PlayerInputPacket();
            packet.inputIndex = Serializer.Deserialize_u16(byteArray, ref offset);
            packet.inputs = PlayerInput.Deserialize(byteArray, ref offset);

            return packet;
        }
    }

    public class PlayerConnectPacket : BasePacket
    {
        public PlayerConnectPacket() { Opcode = EOpcode.S_PlayerConnected; }

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
            packet.ready = Serializer.Deserialize_uByte(byteArray, ref offset) != 0;

            return packet;
        }
    }

    public class PlayerDisconnectedPacket : BasePacket
    {
        public PlayerDisconnectedPacket() { Opcode = EOpcode.S_PlayerDisconnected; }

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
        public ReadyPacket() { Opcode = EOpcode.S_Ready; }

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
            packet.ready = Serializer.Deserialize_uByte(byteArray, ref offset) != 0;

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
                player.ready = Serializer.Deserialize_uByte(byteArray, ref offset) != 0;

                packet.playerList.Add(player);
            }

            return packet;
        }
    }

    public class GameStateRunningPacket : BasePacket
    {
        public GameStateRunningPacket() { Opcode = EOpcode.S_RunningState; }

        public struct RunningPacketData
        {
            public UInt16 playerIndex;
            public byte slotId;
            public bool isInfected;
        };

        public List<RunningPacketData> playerList = new List<RunningPacketData>();

        public override void Serialize(List<byte> byteArray)
        {
            Serializer.Serialize_uByte(byteArray, (byte)playerList.Count);

            foreach (RunningPacketData data in playerList)
            {
                Serializer.Serialize_u16(byteArray, data.playerIndex);
                byte other = (byte)(data.isInfected ? 0b10000000 : 0b0);
                other |= data.slotId;

                Serializer.Serialize_uByte(byteArray, other);
            }
        }

        public static GameStateRunningPacket Deserialize(List<byte> byteArray, ref int offset)
        {
            GameStateRunningPacket packet = new GameStateRunningPacket();

            int count = Serializer.Deserialize_uByte(byteArray, ref offset);
            for (int i = 0; i < count; ++i)
            {
                RunningPacketData data = new RunningPacketData();
                data.playerIndex = Serializer.Deserialize_u16(byteArray, ref offset);
                byte other = Serializer.Deserialize_uByte(byteArray, ref offset);
                data.isInfected = (other & 0b10000000) != 0;
                data.slotId = (byte)(other & 0b01111111);

                packet.playerList.Add(data);
            }

            return packet;
        }
    }

    public class GameStateStartMovePacket : BasePacket
    {
        GameStateStartMovePacket() { Opcode = EOpcode.S_StartMovingState; }

        public bool moveInfected;

        public override void Serialize(List<byte> byteArray)
        {
            Serializer.Serialize_uByte(byteArray, (byte)(moveInfected ? 1 : 0));
        }

        public static GameStateStartMovePacket Deserialize(List<byte> byteArray, ref int offset)
        {
            GameStateStartMovePacket packet = new GameStateStartMovePacket();
            packet.moveInfected = Serializer.Deserialize_uByte(byteArray, ref offset) != 0;

            return packet;
        }
    }

    public class GameStateStartPacket : BasePacket
    {
        public GameStateStartPacket() { Opcode = EOpcode.S_StartGameState; }

        public UInt32 gameDuration;

        public override void Serialize(List<byte> byteArray)
        {
            Serializer.Serialize_u32(byteArray, gameDuration);
        }

        public static GameStateStartPacket Deserialize(List<byte> byteArray, ref int offset)
        {
            GameStateStartPacket packet = new GameStateStartPacket();
            packet.gameDuration = Serializer.Deserialize_u32(byteArray, ref offset);

            return packet;
        }
    }

    public class GameStateFinishPacket : BasePacket
    {
        GameStateFinishPacket() { Opcode = EOpcode.S_FinishedState; }

        public bool infectedWins;

        public override void Serialize(List<byte> byteArray)
        {
            Serializer.Serialize_uByte(byteArray, (byte)(infectedWins ? 1 : 0));
        }

        public static GameStateFinishPacket Deserialize(List<byte> byteArray, ref int offset)
        {
            GameStateFinishPacket packet = new GameStateFinishPacket();
            packet.infectedWins = Serializer.Deserialize_uByte(byteArray, ref offset) != 0;

            return packet;
        }
    }

    public class PlayersStatePacket : BasePacket
    {
        PlayersStatePacket() { Opcode = EOpcode.S_PlayersState; }

        public struct PlayerState
        {
            public UInt16 playerIndex;
            public PlayerInput inputs;
            public float turnAngle;

            public PhysicState physicState;
            public bool atRest;
        };

        public List<PlayerState> otherPlayerState = new List<PlayerState>();

        // Prediction / Reconciliation
        public UInt16 inputIndex;
        public float localTurnAngle;
        public PhysicState localPhysicState = new PhysicState();
        public bool localAtRest;

        public override void Serialize(List<byte> byteArray)
        {
            Serializer.Serialize_u16(byteArray, inputIndex);
            Serializer.Serialize_float(byteArray, localTurnAngle);

            Serializer.Serialize_float(byteArray, localPhysicState.position.x);
            Serializer.Serialize_float(byteArray, localPhysicState.position.y);
            Serializer.Serialize_float(byteArray, localPhysicState.position.z);

            Serializer.Serialize_float(byteArray, localPhysicState.rotation.x);
            Serializer.Serialize_float(byteArray, localPhysicState.rotation.y);
            Serializer.Serialize_float(byteArray, localPhysicState.rotation.z);
            Serializer.Serialize_float(byteArray, localPhysicState.rotation.w);

            Serializer.Serialize_uByte(byteArray, (byte)(localAtRest ? 1 : 0));
            if (!localAtRest)
            {
                Serializer.Serialize_float(byteArray, localPhysicState.linearVelocity.x);
                Serializer.Serialize_float(byteArray, localPhysicState.linearVelocity.y);
                Serializer.Serialize_float(byteArray, localPhysicState.linearVelocity.z);

                Serializer.Serialize_float(byteArray, localPhysicState.angularVelocity.x);
                Serializer.Serialize_float(byteArray, localPhysicState.angularVelocity.y);
                Serializer.Serialize_float(byteArray, localPhysicState.angularVelocity.z);

                Serializer.Serialize_float(byteArray, localPhysicState.frontLeftWheelVelocity);
                Serializer.Serialize_float(byteArray, localPhysicState.frontRightWheelVelocity);
                Serializer.Serialize_float(byteArray, localPhysicState.rearLeftWheelVelocity);
                Serializer.Serialize_float(byteArray, localPhysicState.rearRightWheelVelocity);
            }

            Serializer.Serialize_uByte(byteArray, (byte)otherPlayerState.Count);
            foreach(PlayerState player in otherPlayerState)
            {
                Serializer.Serialize_u16(byteArray, player.playerIndex);
                player.inputs.Serialize(byteArray);
                Serializer.Serialize_float(byteArray, player.turnAngle);

                Serializer.Serialize_float(byteArray, player.physicState.position.x);
                Serializer.Serialize_float(byteArray, player.physicState.position.y);
                Serializer.Serialize_float(byteArray, player.physicState.position.z);

                Serializer.Serialize_float(byteArray, player.physicState.rotation.x);
                Serializer.Serialize_float(byteArray, player.physicState.rotation.y);
                Serializer.Serialize_float(byteArray, player.physicState.rotation.z);
                Serializer.Serialize_float(byteArray, player.physicState.rotation.w);

                Serializer.Serialize_uByte(byteArray, (byte)(player.atRest ? 1 : 0));
                if(!player.atRest)
                {
                    Serializer.Serialize_float(byteArray, player.physicState.linearVelocity.x);
                    Serializer.Serialize_float(byteArray, player.physicState.linearVelocity.y);
                    Serializer.Serialize_float(byteArray, player.physicState.linearVelocity.z);

                    Serializer.Serialize_float(byteArray, player.physicState.angularVelocity.x);
                    Serializer.Serialize_float(byteArray, player.physicState.angularVelocity.y);
                    Serializer.Serialize_float(byteArray, player.physicState.angularVelocity.z);

                    Serializer.Serialize_float(byteArray, player.physicState.frontLeftWheelVelocity);
                    Serializer.Serialize_float(byteArray, player.physicState.frontRightWheelVelocity);
                    Serializer.Serialize_float(byteArray, player.physicState.rearLeftWheelVelocity);
                    Serializer.Serialize_float(byteArray, player.physicState.rearRightWheelVelocity);
                }
            }
        }

        public static PlayersStatePacket Deserialize(List<byte> byteArray, ref int offset)
        {
            PlayersStatePacket packet = new PlayersStatePacket();

            packet.inputIndex = Serializer.Deserialize_u16(byteArray, ref offset);
            packet.localTurnAngle = Serializer.Deserialize_float(byteArray, ref offset);

            packet.localPhysicState.position.x = Serializer.Deserialize_float(byteArray, ref offset);
            packet.localPhysicState.position.y = Serializer.Deserialize_float(byteArray, ref offset);
            packet.localPhysicState.position.z = Serializer.Deserialize_float(byteArray, ref offset);

            packet.localPhysicState.rotation.x = Serializer.Deserialize_float(byteArray, ref offset);
            packet.localPhysicState.rotation.y = Serializer.Deserialize_float(byteArray, ref offset);
            packet.localPhysicState.rotation.z = Serializer.Deserialize_float(byteArray, ref offset);
            packet.localPhysicState.rotation.w = Serializer.Deserialize_float(byteArray, ref offset);

            packet.localAtRest = Serializer.Deserialize_uByte(byteArray, ref offset) != 0;
            if (!packet.localAtRest)
            {
                packet.localPhysicState.linearVelocity.x = Serializer.Deserialize_float(byteArray, ref offset);
                packet.localPhysicState.linearVelocity.y = Serializer.Deserialize_float(byteArray, ref offset);
                packet.localPhysicState.linearVelocity.z = Serializer.Deserialize_float(byteArray, ref offset);

                packet.localPhysicState.angularVelocity.x = Serializer.Deserialize_float(byteArray, ref offset);
                packet.localPhysicState.angularVelocity.y = Serializer.Deserialize_float(byteArray, ref offset);
                packet.localPhysicState.angularVelocity.z = Serializer.Deserialize_float(byteArray, ref offset);

                packet.localPhysicState.frontLeftWheelVelocity = Serializer.Deserialize_float(byteArray, ref offset);
                packet.localPhysicState.frontRightWheelVelocity = Serializer.Deserialize_float(byteArray, ref offset);
                packet.localPhysicState.rearLeftWheelVelocity = Serializer.Deserialize_float(byteArray, ref offset);
                packet.localPhysicState.rearRightWheelVelocity = Serializer.Deserialize_float(byteArray, ref offset);
            }

            int playerCount = Serializer.Deserialize_uByte(byteArray, ref offset);
            for(int i = 0; i < playerCount; ++i)
            {
                PlayerState player = new PlayerState();
                player.playerIndex = Serializer.Deserialize_u16(byteArray, ref offset);
                player.inputs = PlayerInput.Deserialize(byteArray, ref offset);
                player.turnAngle = Serializer.Deserialize_float(byteArray, ref offset);

                player.physicState = new PhysicState();
                player.physicState.position.x = Serializer.Deserialize_float(byteArray, ref offset);
                player.physicState.position.y = Serializer.Deserialize_float(byteArray, ref offset);
                player.physicState.position.z = Serializer.Deserialize_float(byteArray, ref offset);

                player.physicState.rotation.x = Serializer.Deserialize_float(byteArray, ref offset);
                player.physicState.rotation.y = Serializer.Deserialize_float(byteArray, ref offset);
                player.physicState.rotation.z = Serializer.Deserialize_float(byteArray, ref offset);
                player.physicState.rotation.w = Serializer.Deserialize_float(byteArray, ref offset);

                player.atRest = Serializer.Deserialize_uByte(byteArray, ref offset) != 0;
                if (!player.atRest)
                {
                    player.physicState.linearVelocity.x = Serializer.Deserialize_float(byteArray, ref offset);
                    player.physicState.linearVelocity.y = Serializer.Deserialize_float(byteArray, ref offset);
                    player.physicState.linearVelocity.z = Serializer.Deserialize_float(byteArray, ref offset);

                    player.physicState.angularVelocity.x = Serializer.Deserialize_float(byteArray, ref offset);
                    player.physicState.angularVelocity.y = Serializer.Deserialize_float(byteArray, ref offset);
                    player.physicState.angularVelocity.z = Serializer.Deserialize_float(byteArray, ref offset);

                    player.physicState.frontLeftWheelVelocity = Serializer.Deserialize_float(byteArray, ref offset);
                    player.physicState.frontRightWheelVelocity = Serializer.Deserialize_float(byteArray, ref offset);
                    player.physicState.rearLeftWheelVelocity = Serializer.Deserialize_float(byteArray, ref offset);
                    player.physicState.rearRightWheelVelocity = Serializer.Deserialize_float(byteArray, ref offset);
                }

                packet.otherPlayerState.Add(player);
            }

            return packet;
        }
    }

    public class PlayerInfectedPacket : BasePacket
    {
        public PlayerInfectedPacket() { Opcode = EOpcode.S_PlayerInfected; }

        public UInt16 playerIndex;

        public override void Serialize(List<byte> byteArray)
        {
            Serializer.Serialize_u16(byteArray, playerIndex);
        }

        public static PlayerInfectedPacket Deserialize(List<byte> byteArray, ref int offset)
        {
            PlayerInfectedPacket packet = new PlayerInfectedPacket();
            packet.playerIndex = Serializer.Deserialize_u16(byteArray, ref offset);

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
