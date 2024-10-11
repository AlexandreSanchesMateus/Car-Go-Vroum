using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Player
{
    public Player(int id, string name)
    {
        Index = id;
        Name = name;
    }

    public int Index { get; private set; }
    public string Name { get; private set; }

    public bool ready;
}
