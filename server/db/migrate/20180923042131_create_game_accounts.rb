class CreateGameAccounts < ActiveRecord::Migration[5.2]
  def change
    create_table :game_accounts do |t|
      t.reference :account_id
      t.reference :character_id
      t.int :matches_played
      t.int :winning_matches

      t.timestamps
    end
  end
end
