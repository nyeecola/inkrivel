class CreateCharacters < ActiveRecord::Migration[5.2]
  def change
    create_table :characters do |t|
      t.string :name
      t.integer :hit_points
      t.integer :damage
      t.float :attack_speed
      t.float :speed
      t.string :model_file
      t.string :texture_file
      t.integer :hitbox_radius

      t.timestamps
    end
  end
end
