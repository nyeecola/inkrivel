Rails.application.routes.draw do
  resources :game_accounts
  resources :characters
  resources :maps

  resources :accounts do
    post :connect, on: :collection
    post :disconnect, on: :collection
  end

end
