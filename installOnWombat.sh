echo "deb [trusted=yes] https://git.tech-support.systems/TechSupport/game-state/raw/branch/apt-repository ./" | sudo tee /etc/apt/sources.list.d/botball.list
sudo apt update
sudo apt install botball-gamestate
