class CreateGames < ActiveRecord::Migration[5.2]
  def change
    create_table :games do |t|
      t.references :map, foreign_key: true
      t.integer :room_size
      t.string :state
      t.string :string

      t.timestamps
    end
  end
end
