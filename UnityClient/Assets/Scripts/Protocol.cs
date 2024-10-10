using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using ENet6;
using System.Net;
using System.Runtime.InteropServices;


namespace NetworkProtocol
{
    public enum EOpcode : byte
    {
        C_PlayerName,
    }

    public class Protocol
    {

    }

    public class Serializer
    {
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
