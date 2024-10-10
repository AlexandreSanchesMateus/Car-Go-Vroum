using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "NewGameRefSCO", menuName = "ScriptableObjects/GameRefSCO", order = 1)]
public class GameRefSCO : ScriptableObject
{
    public NetworkManager network;
    public MenuManager Menu;
    public GameManager Game;
}
