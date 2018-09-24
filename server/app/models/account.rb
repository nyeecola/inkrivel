class Account < ApplicationRecord

  has_secure_password

  has_one :game_account
  has_one :character, through: :game_account

  validates :user, presence: true, uniqueness: { case_sensitive: false }
  validates :nickname, presence: true
  validates :email, presence: true, uniqueness: { case_sensitive: false }


  def connect(password)
    valid = authenticate(password)
    update(connected: true, last_login: Time.now) if valid
    valid
  end


  def disconnect(password)
    valid = authenticate(password)
    update(connected: false) if valid
    valid
  end

end
