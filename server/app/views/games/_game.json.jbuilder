json.extract! game, :id, :map_id, :room_size, :integer, :state, :string, :created_at, :updated_at
json.url game_url(game, format: :json)
