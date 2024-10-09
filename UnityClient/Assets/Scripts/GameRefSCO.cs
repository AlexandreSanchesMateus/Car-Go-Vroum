using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "NewGameRefSCO", menuName = "ScriptableObjects/GameRefSCO", order = 1)]
public class GameRefSCO : ScriptableObject
{
    public NetworkManager Network {  get; set; }
    public MenuManager Menu { get; set; }
    public GameManager Game { get; set; }
}
