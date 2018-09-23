json.extract! account, :id, :user, :email, :nickname, :password_digest, :last_login, :created_at, :updated_at
json.url account_url(account, format: :json)
