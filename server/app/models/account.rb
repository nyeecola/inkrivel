class Account < ApplicationRecord

  has_secure_password

  has_one :game_account

end
