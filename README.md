<div align="center">
    <h3>A metamod plugin for Surf Combat CS2</h3>
</div>

#### Requirements
- [Metamod:Source](https://www.sourcemm.net/downloads.php/?branch=master) (build 1219 or higher)

#### Win-64

```sh
git clone https://github.com/mEldevlp/surfcombat-metamod-cs2.git
cd surfcombat-metamod-cs2
git submodule update --init --recursive
```

#### Linux
```sh
git clone https://github.com/mEldevlp/surfcombat-metamod-cs2.git
cd surfcombat-metamod-cs2
git submodule update --init --recursive
mkdir build && cd build
CC=gcc CXX=g++ python3 ../configure.py --hl2sdk-root "../" -s cs2
ambuild
```

#### Features

- [x] Red and Blue player models
- [x] Spawn protection 10 seconds
- [x] Commands
    - [x] !hide - teammates hide
    - [x] !legs - hide player legs
    - [x] !rs - reset score
- [x] Admin
    - [x] Each kill gives you 10-5 HP with green(or any) screen fade (and mb force reload gun without anim)
    - [x] !kick <steam_id> or <user_name> \<reason>
    - [x] !ban <steam_id> or <user_name> \<reason> <time in seconds (-1 for perm)>
    - [x] !gag <steam_id> or <user_name> \<reason> \<time in seconds>
    - [x] !map \<map name>
- [x] Rock The Vote
- [x] Chat player prefix
- [x] Rank System (Silver-I to Global Elite)
    - [x] Rank will shows in tab or as prefix in chat
    - [x] Any benefits of achieving a high rank
    - [x] Rating leaderboard?
- [x] Events
    - [x] It will show how much damage you have done (print alert or chat)
    - [x] After death, it will show who you killed and how much damage you did
- [x] !ws?
- [x] Purchase of privileges via GameCMS or tg bot
- [ ] ~~User Menu?~~

