json.extract! account, :id, :user, :email, :nickname, :last_login, :created_at
json.url account_url(account, format: :json)
