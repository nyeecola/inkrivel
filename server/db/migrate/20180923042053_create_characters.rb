class CreateCharacters < ActiveRecord::Migration[5.2]
  def change
    create_table :characters do |t|
      t.string :name
      t.int :hit_points
      t.int :damage
      t.double :attack_speed
      t.double :speed
      t.string :model_file
      t.string :texture_file
      t.int :hitbox_radius

      t.timestamps
    end
  end
end
