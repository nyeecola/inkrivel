class CreateMaps < ActiveRecord::Migration[5.2]
  def change
    create_table :maps do |t|
      t.string :name
      t.string :model_file
      t.string :texture_file

      t.timestamps
    end
  end
end
