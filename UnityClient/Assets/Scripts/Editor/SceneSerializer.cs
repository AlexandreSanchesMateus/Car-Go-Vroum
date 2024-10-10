using System;
using System.Collections.Generic;
using System.IO;
using Unity.Plastic.Newtonsoft.Json;
using UnityEditor;
using UnityEngine;
using UnityEngine.SceneManagement;

public class SceneSerializer : EditorWindow
{
    [MenuItem("Scene Serializer/Serialize Current Scene")]
    public static void SerializeScene()
    {
        Scene scene = SceneManager.GetActiveScene();
        MeshFilter[] meshFilterList = FindObjectsOfType<MeshFilter>();
        SceneData sceneData = new SceneData();
        foreach (var meshFilter in meshFilterList)
        {
            switch (meshFilter.GetComponent<Collider>())
            {
                case SphereCollider:
                    sceneData.SceneObjects.Add(SphereToData(meshFilter.gameObject));
                    break;
                case BoxCollider:
                    sceneData.SceneObjects.Add(BoxToData(meshFilter.gameObject));
                    break;
                case CapsuleCollider:
                    sceneData.SceneObjects.Add(CapsuleToData(meshFilter.gameObject));
                    break;
                case MeshCollider:
                    sceneData.SceneObjects.Add(MeshToData(meshFilter.sharedMesh));
                    break;
            }
                
        }

        string jsonOutput = JsonConvert.SerializeObject(sceneData, Formatting.Indented);
        string path = Application.dataPath + "/Export/" + scene.name + ".json";
        try
        {
            File.WriteAllText(path, jsonOutput);
        }
        catch (Exception e)
        {
            Debug.LogError(e.Message);
        }
        finally
        {
            Debug.Log("Scene " + scene.name + " as been successfully serialized, JSON file as been created under Assets/Export");
        }
        
        
    }

    static SphereObject SphereToData(GameObject obj)
    {
        SphereCollider collider = obj.GetComponent<SphereCollider>();
        
        return new SphereObject
        {
            Type = "sphere",
            Position = (obj.transform.position + collider.center).ToFloat3(),
            radius = (collider.radius * obj.transform.localScale).ToFloat3()
        };
    }

    static BoxObject BoxToData(GameObject obj)
    {
        BoxCollider collider = obj.GetComponent<BoxCollider>();

        return new BoxObject
        {
            Type = "box",
            Position = (obj.transform.position + collider.center).ToFloat3(),
            Rotation = obj.transform.rotation.ToFloat4(),
            Scale = obj.transform.localScale.Multiply(collider.size).ToFloat3()
        };
    }

    static CapsuleObject CapsuleToData(GameObject obj)
    {
        CapsuleCollider collider = obj.GetComponent<CapsuleCollider>();
        
        return new CapsuleObject
        {
            Type = "capsule",
            Position = (obj.transform.position + collider.center).ToFloat3(),
            Rotation = obj.transform.rotation.ToFloat4(),
            radius = collider.radius * Math.Max(obj.transform.localScale.x, obj.transform.localScale.z),
        };
    }

    static MeshObject MeshToData(Mesh mesh)
    {
        Float3[] vertices = new Float3[mesh.vertices.Length];
        
        for (int i = 0; i < mesh.vertices.Length; i++)
        {
            vertices[i] = mesh.vertices[i].ToFloat3();
        }
        
        return new MeshObject
        {
            Type = "mesh",
            vertices = vertices,
            triangles =  mesh.triangles
        };
    }
}

[Serializable]
public class SceneData
{
    public List<ObjectData> SceneObjects = new List<ObjectData>();
}

[Serializable]
public class ObjectData
{
    public string Type;
}

[Serializable]
public class SphereObject : ObjectData
{
    public Float3 Position;
    public Float3 radius;
}

[Serializable]
public class BoxObject : ObjectData
{
    public Float3 Position;
    public Float4 Rotation;
    public Float3 Scale;
}

[Serializable]
public class CapsuleObject : ObjectData
{
    public Float3 Position;
    public Float4 Rotation;
    public float radius;
    public float height;
}

[Serializable]
public class MeshObject : ObjectData
{
    public Float3[] vertices;
    public int[] triangles;
}

public static class MathExtensions
{
    public static Vector3 Multiply(this Vector3 v1, Vector3 v2)
    {
        return new Vector3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
    }

    public static Float3 ToFloat3(this Vector3 v)
    {
        return new Float3 { X = v.x, Y = v.y, Z = v.z };
    }
    
    public static Float4 ToFloat4(this Quaternion q)
    {
        return new Float4 { X = q.x, Y = q.y, Z = q.z, W = q.w };
    }
}

[Serializable]
public struct Float3
{
    public float X, Y, Z;
}

public struct Float4
{
    public float X, Y, Z, W;
}
