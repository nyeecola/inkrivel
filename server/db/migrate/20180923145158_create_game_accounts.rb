class CreateGameAccounts < ActiveRecord::Migration[5.2]
  def change
    create_table :game_accounts do |t|
      t.references :account, foreign_key: true
      t.references :character, foreign_key: true
      t.integer :matches_played
      t.integer :winning_matches

      t.timestamps
    end
  end
end
