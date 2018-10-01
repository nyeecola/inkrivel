class GamePlayer < ApplicationRecord
  belongs_to :game
  belongs_to :game_account
end
