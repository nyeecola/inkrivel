json.extract! game_account, :id, :account_id, :character_id, :matches_played, :winning_matches, :created_at, :updated_at
json.url game_account_url(game_account, format: :json)
