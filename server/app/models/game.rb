class Game < ApplicationRecord
  belongs_to :map
  has_many :game_players, :dependent => :destroy

  scope :accepting_players, -> (players) {
    where(room_size: players, state: 'waiting')
        .joins("LEFT OUTER JOIN game_players on games.id = game_players.game_id")
        .group("games.id")
        .having("count(game_players.id) < ?", players)
  }


  def ready_to_start?
    game_players.length == room_size && state == 'waiting'
  end


  def game_ready?
    game_players.where(state: 'starting').length == room_size && state == 'starting'
  end


  def start
    update(state: 'playing')
    game_players.update_all(state: 'playing')
    start_game_server
  end


  def end_game
    update(state: 'finished')
    game_players.update_all(state: 'finished')
  end


  def required_players
    if state == 'starting'
      result = room_size - game_players.where(state: 'starting').length
    else
      result = room_size - game_players.length
    end

    result
  end


  def next_player_id
    if game_players.count > 0
      result = game_players.maximum(:player_id) + 1
    else
      result = 0
    end
    result
  end


  private

  def start_game_server
    Thread.new {
      characters = game_players.order(player_id: :asc).map { |player| player.game_account.character_id }
      %x(./../game_server/bin/game_server #{room_size} #{ characters.join(' ') })
      end_game
    }
  end
end
