class CreateGamePlayers < ActiveRecord::Migration[5.2]
  def change
    create_table :game_players do |t|
      t.references :game, foreign_key: true
      t.references :game_account, foreign_key: true
      t.string :state
      t.integer :player_id

      t.timestamps
    end
  end
end
