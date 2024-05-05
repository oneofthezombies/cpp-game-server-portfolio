# Rock Paper Scissors Lizard Spock 🖖

This project was created to prototype the [Kero Modular Server Engine](../../README.md)

## How to Play

### Build

Build it by running `python dev.py build` in the repository root.  
Please check the repository [README](../../README.md) for more details.  

### Server

Please set the `port` to communicate with the client.

```sh
build/examples/rock_paper_scissors_lizard_spock/server --port 8000
```

### Client

Please set the `ip` and `port` arguments according to the server address.

```sh
build/examples/rock_paper_scissors_lizard_spock/client --ip 127.0.0.1 --port 8000
```

When two clients connect, a _battle_ begins.  

```sh
[DEBUG] Server: {"socket_id":9,"event":"connect"} (33bytes)
Connected to the server with socket_id: 9
[DEBUG] Server: {"opponent_socket_id":8,"battle_id":1,"event":"battle_start"} (61bytes)
Battle started with battle id: 1 and opponent socket id: 8
Please enter your action: Available actions: rock, paper, scissors, lizard, spock
```

Enter `rock`, `paper`, `scissors`, `lizard`, or `spock` 🖖.  
When both players have completed their input, the results are printed.  

```sh
spock
Move sent to the server. Waiting for the opponent's action.
[DEBUG] Server: {"result":2,"event":"battle_result"} (36bytes)
Battle result: lose
You lost the battle.
```

Oh my, I lost.  

## Reference

Rock, Paper, Scissors, Lizard, Spock - https://bigbangtheory.fandom.com/wiki/Rock,_Paper,_Scissors,_Lizard,_Spock
