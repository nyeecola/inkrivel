class Map < ApplicationRecord

  has_many :games

  def self.random
    all.order("RANDOM()").first
  end

end
