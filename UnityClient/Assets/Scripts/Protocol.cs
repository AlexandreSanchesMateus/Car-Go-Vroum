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
        public static void Serialize_sByte(ref List<byte> byteArray, sbyte value)
        {
            Serialize_uByte(ref byteArray, (byte)value);
        }

        public static void Serialize_sByte(ref List<byte> byteArray, ref int offset, sbyte value)
        {
            Serialize_uByte(ref byteArray, ref offset, (byte)value);
        }

        public static void Serialize_i16(ref List<byte> byteArray, Int16 value)
        {
            Serialize_u16(ref byteArray, (UInt16)value);
        }

        public static void Serialize_i16(ref List<byte> byteArray, ref int offset, Int16 value)
        {
            Serialize_u16(ref byteArray, ref offset, (UInt16)value);
        }

        public static void Serialize_i32(ref List<byte> byteArray, Int32 value)
        {
            Serialize_u32(ref byteArray, (UInt32)value);
        }

        public static void Serialize_i32(ref List<byte> byteArray, ref int offset, Int32 value)
        {
            Serialize_u32(ref byteArray, ref offset, (UInt32)value);
        }

        public static void Serialize_i64(ref List<byte> byteArray, Int64 value)
        {
            Serialize_u64(ref byteArray, (UInt64)value);
        }

        public static void Serialize_i64(ref List<byte> byteArray, ref int offset, Int64 value)
        {
            Serialize_u64(ref byteArray, ref offset, (UInt64)value);
        }

        public static void Serialize_uByte(ref List<byte> byteArray, byte value)
        {
            int offset = byteArray.Count;
            byteArray.Capacity = offset + sizeof(byte);
            Serialize_uByte(ref byteArray, ref offset, (byte)value);
        }

        public static void Serialize_uByte(ref List<byte> byteArray, ref int offset, byte value)
        {
            byteArray[offset] = value;
        }

        public static void Serialize_u16(ref List<byte> byteArray, UInt16 value)
        {
            int offset = byteArray.Count;
            byteArray.Capacity = offset + sizeof(UInt16);
            Serialize_u16(ref byteArray, ref offset, value);
        }

        public static void Serialize_u16(ref List<byte> byteArray, ref int offset, UInt16 value)
        {
            byte[] networkValue = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(value));
            for (int i = offset; i < offset + sizeof(UInt16); ++i)
            {

            }
        }

        public static void Serialize_u32(ref List<byte> byteArray, UInt32 value)
        {

        }

        public static void Serialize_u32(ref List<byte> byteArray, ref int offset, UInt32 value)
        {

        }

        public static void Serialize_u64(ref List<byte> byteArray, UInt64 value)
        {

        }

        public static void Serialize_u64(ref List<byte> byteArray, ref int offset, UInt64 value)
        {

        }
    }
}
