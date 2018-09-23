class CreateAccounts < ActiveRecord::Migration[5.2]
  def change
    create_table :accounts do |t|
      t.string :user
      t.string :email
      t.string :nickname
      t.string :password_digest
      t.date :last_login

      t.timestamps
    end
  end
end
