class GameAccount < ApplicationRecord

  belongs_to :account

  has_many :game_players

end
