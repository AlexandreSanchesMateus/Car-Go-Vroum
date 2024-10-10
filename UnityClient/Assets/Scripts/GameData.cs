using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GameData
{
    public enum GameState
    {
        WAITING,         // attendre que tout les joueurs soient prets
        RUNNING,         // la partie est en cours
        FINISHED         // une des deux équipes a gagné
    }

    public GameState state;
    public List<Player> players = new List<Player>();

    public int ownPlayerIndex;
}
