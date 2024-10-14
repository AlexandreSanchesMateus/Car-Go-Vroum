using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GameData
{
    public enum GameState
    {
        LOBBY,
        WAITING_GAME_START,
        GAME_STARTED,
        GAME_FINISHED
    }

    public GameState state = GameData.GameState.LOBBY;
    public List<Player> players = new List<Player>();
    public int ownPlayerIndex = -1;
}
