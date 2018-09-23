class GameAccount < ApplicationRecord
  belongs_to :account
  belongs_to :character
end
