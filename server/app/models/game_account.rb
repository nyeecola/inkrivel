class GameAccount < ApplicationRecord

  belongs_to :account
  belongs_to :character, optional: true

  has_many :game_players

end
