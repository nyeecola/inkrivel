Rails.application.routes.draw do

  resources :games do
    post :join, on: :collection
    post :leave, on: :collection
    get :state, on: :collection
  end

  resources :game_accounts
  resources :characters
  resources :maps

  resources :accounts do
    post :connect, on: :collection
    post :disconnect, on: :collection
  end

end
