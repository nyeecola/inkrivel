class CreateGameAccounts < ActiveRecord::Migration[5.2]
  def change
    create_table :game_accounts do |t|
      t.references :account, foreign_key: true
      t.integer :character_id, default: 0
      t.integer :matches_played, default: 0
      t.integer :winning_matches, default: 0

      t.timestamps
    end
  end
end
